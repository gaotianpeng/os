#ifndef __FS_INODE_H
#define __FS_INODE_H

#include "stdint.h"
#include "list.h"
#include "ide.h"

struct inode {
    uint32_t i_no;          // inode 编号
    
    /* 
        当此inode是文件时, i_size是指文件大小,
        若此inode是目录, i_size是指该目录下所有目录项大小之和
    */
    uint32_t i_size;

    uint32_t i_open_cnts;   // 记录此文件被打开的次数, 在关闭文件时，回收与之相关的资源时使用
    bool write_deny;        // 写文件不能并行, 进程写文件前检查此标识
    // i_sectors[0-11]是直接块, i_sectors[12]用来存储一级间接块指针。支持140个块(扇区)大小的文件
    uint32_t i_sectors[13];
    struct list_elem inode_tag; // 用于加入“已打开的inode列表”内存缓存
};

struct inode* inode_open(struct partition* part, uint32_t inode_no);
void inode_sync(struct partition* part, struct inode* inode, void* io_buf);
void inode_init(uint32_t inode_no, struct inode* new_inode);
void inode_close(struct inode* inode);


#endif // __FS_INODE_H
