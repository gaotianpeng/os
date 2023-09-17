#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H

#include "stdint.h"
#include "list.h"
#include "bitmap.h"
#include "memory.h"


#define MAX_FILES_OPEN_PER_PROC 8


typedef void thread_func(void*);
typedef int16_t pid_t;

// 进程或线程的状态
enum task_status {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED
};

/*
    中断栈
        此结构用于中断发生时保护程序（进程或线程）的上下文环境
        进程或线程被外部中断或软件中断打断时，会按照此结构压入上下文寄存器
        intr_exit中的出栈操作是此结构的逆操作
        此栈在线程自己的内核栈中位置固定：所在页的最顶端    
*/
struct intr_stack {
    uint32_t vec_no;        // kernel中 宏VECTOR中push %1压入的中断号
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;     // 虽然pushad把esp也压入,但esp是不断变化的,所以会被popad忽略
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    // 以下由CPU从低特权级进入高特权级时压入
    uint32_t err_code;      // err_code 会被压入在eip之后
    void (*eip)(void);
    uint32_t cs;
    uint32_t eflags;
    void* esp;
    uint32_t ss;
};

struct thread_stack {
    /*
        ABI规定要备份ebp、ebx、edi、esi、ebp
        esp的值会由调用约定来保证，不保护esp的值
    */
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    /*
        线程第一次执行时，eip指向待调用的函数kernel_thread 
        其它时候，eip是指向switch_to的返回地址
    */
    void (*eip)(thread_func* func, void* func_arg);

    // 以下仅供第一次被调度上cpu时使用
    // 参数unused_ret只为占位置充数为返回地址
    void (*unused_retaddr);
    thread_func* function;  // 由kernel_thread所调用的函数名
    void* func_arg;         // 由kernel_thread所调用的函数所需的参数
};

// 进程或线程的控制块pcb
struct task_struct {
    uint32_t* self_kstack;      // 各内核都有自己的内核栈
    pid_t pid;
    enum task_status status;
    char name[16];
    uint8_t priority;           // 线程优先级
    uint8_t ticks;              // 每次在处理器上执行的时间嘀嗒数

    // 此任务自从在CPU上运行后至今占用了多少CPU嘀嗒数，也就是执行了多久
    uint32_t elapsed_ticks;

    /*
        general_tag的作用是用于线程在一般的队列中的结点
        是线程的标签，当线程被加入到就绪队列thread_ready_list或其他等待队列中时，
        就把该线程PCB中general_tag的地址加入队列
    */
    struct list_elem general_tag;

    // all_list_tag的作用是用于线程队列thread_all_list中的结点
    struct list_elem all_list_tag;

    uint32_t* pgdir;            // 进程自己页表的虚拟地址，线程此属性为NULL

    struct virtual_addr userprog_vaddr; // 用户进程的虚拟地址池
    struct mem_block_desc u_block_desc[DESC_CNT];   // 用户进程内存块描述符
    int32_t fd_table[MAX_FILES_OPEN_PER_PROC];  // 已打开文件数组
    uint32_t cwd_inode_nr;      // 进程所在的工作目录的inode号
    int16_t parent_pid;         // 父进程pid
    uint32_t stack_magic;       // 用这串数字做栈的边界标记，用于检测栈的溢出
};

extern struct list thread_ready_list;
extern struct list thread_all_list;

struct task_struct* running_thread(void);
void thread_create(struct task_struct* pthread, thread_func function,void* func_arg);
void init_thread(struct task_struct* pthread, char* name, int prib);
struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_arg);
void schedule(void);
void thread_init(void);

void thread_block(enum task_status stat);
void thread_unblock(struct task_struct* pthread);

void thread_yield(void);
pid_t fork_pid(void);
void sys_ps(void);

#endif // __THREAD_THREAD_