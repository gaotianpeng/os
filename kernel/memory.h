#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"
#include "list.h"

// 内存池标记，用于判断用使用哪个内存池
enum pool_flags {
    PF_KERNEL = 1,      // 内核内存池
    PF_USER = 2         // 用户内存池
};

#define	 PG_P_1	  1	    // 页表项或页目录项存在属性位，表示此页内存已经存在
#define	 PG_P_0	  0	    // 页表项或页目录项存在属性位，表示此页内存不存在
#define	 PG_RW_R  0	    // R/W 属性位值, 读/执行
#define	 PG_RW_W  2	    // R/W 属性位值, 读/写/执行
#define	 PG_US_S  0	    // U/S 属性位值, 系统级，只允许0、1、2特权级访问
#define	 PG_US_U  4	    // U/S 属性位值, 用户级，允许所有特权级别程序访问

// 用于虚拟地址管理
struct virtual_addr {
    struct bitmap vaddr_bitmap;     // 虚拟地址用到的位图结构，以页为单位管理虚拟地址的分配
    uint32_t vaddr_start;           // 虚拟地址的起始地址
};

// 内存块
struct mem_block {
    struct list_elem free_elem;
};

// 内存块描述符，一个 mem_block_desc 包含多个 arena
struct mem_block_desc {
    uint32_t block_size;            // 内存块大小
    uint32_t blocks_per_arena;      // 本arena中可容纳此mem_block的数量
    struct list free_list;          // 目前可用的 mem_block 链表
};

// 16、32、64、128、256、512、1024
#define DESC_CNT 7	   // 内存块描述符个数

extern int page_table_add_num;
extern struct pool kernel_pool, user_pool;
void mem_init(void);
void* get_kernel_pages(uint32_t pg_cnt);
void* malloc_page(enum pool_flags pf, uint32_t pg_cnt);
void malloc_init(void);
uint32_t* pte_ptr(uint32_t vaddr);
uint32_t* pde_ptr(uint32_t vaddr);
uint32_t addr_v2p(uint32_t vaddr);
void* get_a_page(enum pool_flags pf, uint32_t vaddr);
void* get_user_pages(uint32_t pg_cnt);
void block_desc_init(struct mem_block_desc* desc_array);
void* sys_malloc(uint32_t size);
void mfree_page(enum pool_flags pf, void* _vaddr, uint32_t pg_cnt);
void pfree(uint32_t pg_phy_addr);
void sys_free(void* ptr);
void* get_a_page_without_opvaddrbitmap(enum pool_flags pf, uint32_t vaddr);
void free_a_phy_page(uint32_t pg_phy_addr);

#endif // __LIB_KERNEL_MEMORY_H

