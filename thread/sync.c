#include "sync.h"
#include "list.h"
#include "global.h"
#include "debug.h"
#include "interrupt.h"

// 初始化信号量
void sema_init(struct semaphore* psema, uint8_t value) {
    psema->value = value;
    list_init(&psema->waiters);      // 初始化信号量的等待队列
}

// 初始化锁plock
void lock_init(struct lock* plock) {
    plock->holder = NULL;
    plock->holder_repeat_nr = 0;
    sema_init(&plock->semaphore, 1);    // 信号量初值为1
}

// 信号量down操作
void sema_down(struct semaphore* psema) {
    // 用关中断保证原子操作
    enum intr_status old_status = intr_disable();
    while (psema->value == 0) {     // 若value 为0，表示已经被别人持有
        struct task_struct* cur = running_thread();
        ASSERT(!elem_find(&psema->waiters, &cur->general_tag));
        // 若信号量的值等于0，则当前线程把自己加入该锁的等待队列中，然后阻塞自己
        list_append(&psema->waiters, &cur->general_tag);
        thread_block(TASK_BLOCKED);     // 阻塞线程，直到被唤醒
    }

    psema->value--;
    ASSERT(psema->value == 0);
    intr_set_status(old_status);
}

// 信号量的up操作
void sema_up(struct semaphore* psema) {
    // 关中断，保护原子操作
    enum intr_status old_status = intr_disable();
    ASSERT(psema->value == 0);

    if (!list_empty(&psema->waiters)) {
        struct task_struct* thread_blocked = elem2entry(struct task_struct, general_tag, list_pop(&psema->waiters));
        thread_unblock(thread_blocked);
    }

    psema->value++;
    ASSERT(psema->value == 1);
    // 恢复之前的中断状态
    intr_set_status(old_status);
}

// 获取锁plock
void lock_acquire(struct lock* plock) {
    // 排除曾经自己已经持有锁但还未将其释放的情况
    struct task_struct* cur = running_thread();
    if (plock->holder != cur) {
        sema_down(&plock->semaphore);
        plock->holder = cur;
        ASSERT(plock->holder_repeat_nr == 0);
        plock->holder_repeat_nr = 1;
    } else { // 如果已经持有锁
        plock->holder_repeat_nr++;
    }
}

void lock_release(struct lock* plock) {
    ASSERT(plock->holder == running_thread());
    if (plock->holder_repeat_nr > 1) {
        plock->holder_repeat_nr--;
        return;
    }

    ASSERT(plock->holder_repeat_nr == 1);

    plock->holder = NULL;
    plock->holder_repeat_nr = 0;
    sema_up(&plock->semaphore);
}

