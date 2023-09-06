#ifndef __LIB_KERNEL_MEMORY_H
#define __LIB_KERNEL_MEMORY_H

#include "stdint.h"
#include "bitmap.h"

// 用于虚拟地址管理
struct virtual_addr {
    struct bitmap vaddr_bitmap;     // 虚拟地址用到的位图结构，以页为单位管理虚拟地址的分配
    uint32_t vaddr_start;           // 虚拟地址的起始地址
};

extern struct pool kernel_pool, user_pool;

void mem_init(void);

#endif // __LIB_KERNEL_MEMORY_H