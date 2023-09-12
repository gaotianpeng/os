#ifndef __USERPROG_SYSCALL_INIT_H
#define __USERPROG_SYSCALL_INIT_H

#include "stdint.h"

void syscall_init(void);
uint32_t sys_getpid(void);

#endif // __USERPROG_SYSCALL_INIT_H