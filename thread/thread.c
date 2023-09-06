#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "global.h"
#include "memory.h"

#define PAGE_SIZE 4096

// 由kernel_thread去执行function(func_arg)
static void kernel_thread(thread_func* function, void* func_arg) {
    function(func_arg);
}

// 初始化thread_stack，将待执行的函数和参数放到thread_stack中相应的位置
void thread_create(struct task_struct* pthread, thread_func function, void* func_arg) {
    /*
        先预留中断使用栈的空间
        将来线程进入中断后，kernel中的中断代码会通过此栈保存上下文
        将来实现用户进程时，会将用哀悼进程的初始信息放在中断栈中
        因此，必须事先把intr_stack的空间留出来
    */
    pthread->self_kstack -= sizeof(struct intr_stack);
    // 再预留线程栈空间
    pthread->self_kstack -= sizeof(struct thread_stack);

    struct thread_stack* kthread_stack = (struct thread_stack*)pthread->self_kstack;
    // kernel_stack 不是通过call调用，而是通过ret来执行
    kthread_stack->eip = kernel_thread;
    kthread_stack->function = function;
    kthread_stack->func_arg = func_arg;
    kthread_stack->ebp = kthread_stack->ebx = kthread_stack->esi = 
            kthread_stack->edi = 0;
}

// 初始化线程基本信息
void init_thread(struct task_struct* pthread, char* name, int prio) {
    memset(pthread, 0, sizeof(*pthread));
    strcpy(pthread->name, name);
    pthread->status = TASK_RUNNING;
    pthread->priority = prio;

    // self_kstack 是线程自己在0特权级下所用的栈，在线程创建之初，被初始化为线程PCB的最顶端
    pthread->self_kstack = (uint32_t*)((uint32_t)pthread + PAGE_SIZE);
    pthread->stack_magic = 0x19870916;  // 自定义的魔数
}

struct task_struct* thread_start(char* name, int prio, thread_func funciton,
            void* func_arg) {
    // pcb 都们于内核空间，包括用户进程的pcb也是在内核空间
    struct task_struct* thread = get_kernel_pages(1);
    init_thread(thread, name, prio);
    thread_create(thread, funciton, func_arg);
    // 在执行ret后，处理器会去执行kernel_thread函数。在执行完这句汇编后，线程就会开始执行
    asm volatile ("movl %0, %%esp; \
        pop %%ebp; pop %%ebx; pop %%edi; pop %%esi; ret" : : "g" (thread->self_kstack) : "memory");
    return thread;
}