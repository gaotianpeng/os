#ifndef __FS_DIR_H
#define __FS_DIR_H

#include "stdint.h"
#include "inode.h"
#include "ide.h"
#include "global.h"

// 最大文件名长度, 文件名要存储在目录项中, 目录项大小是固定的, 因此文件名的长度肯定要有个上限
#define MAX_FILE_NAME_LEN  16

/*
    目录结构, 它并不在磁盘上存在，只用于与目录相关的操作时，
    在内存中创建的结构，用过之后就释放了，不会回写到磁盘中
*/
struct dir {
    struct inode* inode;                // “已打开的inode队列”缓存
    uint32_t dir_pos;                   // 记录在目录内的偏移
    uint8_t dir_buf[512];               // 目录的数据缓存，如读取目录时，用来存储返回的目录项
};

struct dir_entry {
    char filename[MAX_FILE_NAME_LEN];   // 普通文件或目录名称
    uint32_t i_no;                      // 普通文件或目录对应的inode编号
    enum file_types f_type;             // 文件类型
};

#endif // __FS_DIR_H