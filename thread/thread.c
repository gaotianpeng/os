#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "global.h"
#include "print.h"
#include "interrupt.h"
#include "memory.h"
#include "debug.h"
#include "process.h"
#include "sync.h"

#define PAGE_SIZE 4096

struct task_struct* main_thread;          // 主线程PCB
struct task_struct* idle_thread;          // idle 线程
struct list thread_ready_list;	         // 就绪队列
struct list thread_all_list;	            // 所有任务队列
struct lock pid_lock;                     // 分配pid锁
static struct list_elem* thread_tag;      // 用于保存队列中的线程结点

extern void switch_to(struct task_struct* cur, struct task_struct* next);


static void idle(void* arg) {
   while (1) {
      thread_block(TASK_BLOCKED);
      // 执行hlt时必须要保证目前处于开中断的情况下
      asm volatile ("sti; hlt" : : : "memory");
   }
}

/*
    获取当前线程PCB指针, 各个线程所用的0级栈指针都是在自己的PCB中
    因此, 取当前栈指针的高20位作为当前运行线程的PCB
*/
struct task_struct* running_thread() {
   uint32_t esp;
   asm ("mov %%esp, %0" : "=g" (esp));
   return (struct task_struct*) (esp & 0xfffff000);
}

// 由kernel_thread去执行function(func_arg)
static void kernel_thread(thread_func* function, void* func_arg) {
   // 执行function前要开中断，避免后面的时钟中断被屏蔽，而无法调度其它线程
   intr_enable();
   function(func_arg);
}

// 分配pid
static pid_t allocate_pid(void) {
   static pid_t next_pid = 0;
   lock_acquire(&pid_lock);
   next_pid++;
   lock_release(&pid_lock);
   return next_pid;
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
   kthread_stack->ebp = kthread_stack->ebx = kthread_stack->edi = kthread_stack->esi = 0;
}

// 初始化线程基本信息
void init_thread(struct task_struct* pthread, char* name, int prio) {
   memset(pthread, 0, sizeof(*pthread));
   pthread->pid = allocate_pid();
   strcpy(pthread->name, name);

   if (pthread == main_thread) {
      // 由于把main函数也封装成一个线程，并且它一直是运行的，故将其直接设为TASK_RUNNING
      pthread->status = TASK_RUNNING;
   } else {
      pthread->status = TASK_READY;
   }

   // self_kstack 是线程自己在0特权级下所用的栈，在线程创建之初，被初始化为线程PCB的最顶端
   pthread->self_kstack = (uint32_t*)((uint32_t)pthread + PAGE_SIZE);
   pthread->priority = prio;
   pthread->ticks = prio;
   pthread->elapsed_ticks = 0;
   pthread->pgdir = NULL;                  // 线程没有自己的地址空间
   pthread->stack_magic = 0x19870916;	    // 自定义的魔数
}

struct task_struct* thread_start(char* name, int prio, thread_func function,
            void* func_arg) {
   // PCB都位于内核空间，包括用户进程的PCB也是在内核空间
   struct task_struct* thread = get_kernel_pages(1);

   init_thread(thread, name, prio);
   thread_create(thread, function, func_arg);

   // 确保之前不在队列中
   ASSERT(!elem_find(&thread_ready_list, &thread->general_tag));
   // 加入就绪线程队列
   list_append(&thread_ready_list, &thread->general_tag);

   // 确保之前不在队列中
   ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag));
   // 加入全部线程队列
   list_append(&thread_all_list, &thread->all_list_tag);

   return thread;
}

// 将kernel中的main函数完善为主线程
static void make_main_thread(void) {
   /* 
      因为main线程早已运行, 在loader中进入内核时的 mov esp, 0xc009f000
      就是为其预留了tcb, 地址为0xc009e000, 因此不需要通过get_kernel_page另分配一页
   */
   main_thread = running_thread();
   init_thread(main_thread, "main", 31);

   /* 
      main函数是当前线程, 当前线程不在thread_ready_list中,
      所以只将其加在thread_all_list中.
   */
   ASSERT(!elem_find(&thread_all_list, &main_thread->all_list_tag));
   list_append(&thread_all_list, &main_thread->all_list_tag);
}

// 将当前线程换下处理器，并在就绪队列中找出下个可运行的程序，将其换上处理器
void schedule() {
   ASSERT(intr_get_status() == INTR_OFF);

   struct task_struct* cur = running_thread(); 
   if (cur->status == TASK_RUNNING) { // 若此线程只是cpu时间片到了，将其加入到就绪队列尾
      ASSERT(!elem_find(&thread_ready_list, &cur->general_tag));
      list_append(&thread_ready_list, &cur->general_tag);
      cur->ticks = cur->priority;     // 重新将当前线程的ticks再重置为其priority;
      cur->status = TASK_READY;
   } else { 
      /* 
         若此线程需要某事件发生后才能继续上cpu运行,
         不需要将其加入队列，因为当前线程不在就绪队列中
      */
   }

   // 如果就绪队列中没有可运行的任务, 就唤醒idle
   if (list_empty(&thread_ready_list)) {
      thread_unblock(idle_thread);
   }

   ASSERT(!list_empty(&thread_ready_list));
   thread_tag = NULL;	  // thread_tag清空
   /*
      thread_tag 并不是线程，它仅仅是线程PCB中的general_tag或all_list_tag
      要获得线程的信息，必须将其转换成PCB才行
   */
   thread_tag = list_pop(&thread_ready_list);

   //(struct task_struct*)((int)thread_tag - (int)(&((struct task_struct*)0)->general_tag))
   struct task_struct* next = elem2entry(struct task_struct, general_tag, thread_tag);
   next->status = TASK_RUNNING;

   process_activate(next);

   switch_to(cur, next);
}

// 当前线程将自己阻塞，标志其状态为stat
void thread_block(enum task_status stat) {
   /*
      stats 取值为 TASK_BLOCKED、TASK_WAITING、TASK_HANGING 时不会被调度
   */
   ASSERT(stat == TASK_BLOCKED || stat == TASK_WAITING
         || stat == TASK_HANGING);

   enum intr_status old_status = intr_disable();
   struct task_struct* cur_thread = running_thread();
   cur_thread->status = stat;
   schedule();           // 将当前线程唤下处理器
   // 待当前线程被解除阻塞后才继续运行
   intr_set_status(old_status);
}

// 将线程pthread解除阻塞
void thread_unblock(struct task_struct* pthread) {
   enum intr_status old_status = intr_disable();
   ASSERT(pthread->status == TASK_BLOCKED || pthread->status == TASK_RUNNING
      || pthread->status == TASK_HANGING);

   if (pthread->status != TASK_READY) {
      ASSERT(!elem_find(&thread_ready_list, &pthread->general_tag));
      if (elem_find(&thread_ready_list, &pthread->general_tag)) {
         PANIC("thread_unblock: blocked thread in thread_list\n");
      }

      // 放到队列的最前面，使其尽快得到调度
      list_push(&thread_ready_list, &pthread->general_tag);
      pthread->status = TASK_READY;
   }

   intr_set_status(old_status);
}

// 主动让出cpu, 换其它线程运行
void thread_yield(void) {
   struct task_struct* cur = running_thread();
   enum intr_status old_status = intr_disable();
   ASSERT(!elem_find(&thread_ready_list, &cur->general_tag));
   list_append(&thread_ready_list, &cur->general_tag);
   cur->status = TASK_READY;
   schedule();
   intr_set_status(old_status);
}

// 初始化线程环境
void thread_init(void) {
   put_str("thread_init start\n");
   list_init(&thread_ready_list);
   list_init(&thread_all_list);
   lock_init(&pid_lock);
   // 将当前main函数创建为线程
   make_main_thread();

   // 创建idle线程
   idle_thread = thread_start("idle", 10, idle, NULL);
   put_str("thread_init done\n");
}



