#ifndef __THREAD_SYNC_H
#define __THREAD_SYNC_H

#include "list.h"
#include "stdint.h"
#include "thread.h"

// 信号量结构
struct semaphore {
    uint8_t value;
    struct list waiters;
};

// 锁结构
struct lock {
    struct task_struct* holder;     // 锁的持有者
    struct semaphore semaphore;     // 用二元信号量实现锁
    /*
        锁的持有者重复申请的次数
            一般情况下我们应该在进入临界区之前加锁，但有时候可能持有了某临界区的锁后，
            在未释放锁之前，有可能会再次调用重复申请此锁的函数
    */
    uint32_t holder_repeat_nr;
};

void sema_init(struct semaphore* psema, uint8_t value);
void lock_init(struct lock* plock);
void sema_down(struct semaphore* psema);
void sema_up(struct semaphore* psema);
void lock_acquire(struct lock* plock);
void lock_release(struct lock* plock);

#endif // __THREAD_SYNC_H