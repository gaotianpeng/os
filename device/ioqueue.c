#include "ioqueue.h"
#include "interrupt.h"
#include "global.h"
#include "debug.h"

void ioqueue_init(struct ioqueue* ioq) {
    lock_init(&ioq->lock);
    ioq->producer = NULL;
    ioq->consumer = NULL;
    ioq->head = ioq->tail = 0;
}

static int32_t next_pos(int32_t pos) {
    return (pos + 1) % bufsize;
}

bool ioq_full(struct ioqueue* ioq) {
    ASSERT(intr_get_status() == INTR_OFF);
    return next_pos(ioq->head) == ioq->tail;
}

bool ioq_empty(struct ioqueue* ioq) {
    ASSERT(intr_get_status() == INTR_OFF);
    return ioq->head == ioq->tail;
}

static void ioq_wait(struct task_strut** waiter) {
    ASSERT(*waiter == NULL && waiter != NULL);
    *waiter = running_thread();
    thread_block(TASK_BLOCKED);
}

static void wakeup(struct task_struct** waiter) {
    ASSERT(*waiter != NULL);
    thread_unblock(*waiter);
    *waiter = NULL;
}

char ioq_getchar(struct ioqueue* ioq) {
    ASSERT(intr_get_status() == INTR_OFF);
    /* 
        若缓冲区(队列)为空，把消费者ioq->consumer记为当前线程自己，
        目的是将来生产者往缓冲区里装商品后，生产者知道唤醒哪个消费者，
        也就是唤醒当前线程自己
    */
    while (ioq_empty(ioq)) {
        lock_acquire(&ioq->lock);
        ioq_wait(&ioq->consumer);
        lock_release(&ioq->lock);
    }

    char byte = ioq->buf[ioq->tail];
    ioq->tail = next_pos(ioq->tail);
    
    if (ioq->producer != NULL) {
        wakeup(&ioq->consumer);
    }

    return byte;
}

void ioq_putchar(struct ioqueue* ioq, char byte) {
    ASSERT(intr_get_status() == INTR_OFF);
    /* 
        若缓冲区(队列)已经满了，把生产者ioq->producer记为自己，
        为的是当缓冲区里的东西被消费者取完后让消费者知道唤醒哪个生产者，
        也就是唤醒当前线程自己
    */
    while (ioq_full(ioq)) {
        lock_acquire(&ioq->lock);
        ioq_wait(&ioq->producer);
        lock_release(&ioq->lock);
    }

    ioq->buf[ioq->head] = byte;
    ioq->head = next_pos(ioq->head);

    if (ioq->consumer != NULL) {
        wakeup(&ioq->consumer);
    }
}
