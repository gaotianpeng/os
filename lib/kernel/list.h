#ifndef __LIB_KERNEL_LIST_H
#define __LIB_KERNEL_LIST_H

#include "global.h"

#define offset(struct_type, member) (int)(&((struct_type*)0)->member)
/* 
    将指针elem_ptr转换成struct_type类型的指针
    原理是用elem_ptr的地址减去elem_ptr在结构体struct_type中的偏移量，
    此地址差便是结构体struct_type的起始地址，最后再将此地址差转换为struct_type指针类型
*/
#define elem2entry(struct_type, struct_member_name, elem_ptr) \
        (struct_type*)((int)elem_ptr - offset(struct_type, struct_member_name))

struct list_elem {
    struct list_elem* prev;
    struct list_elem* next;
};

struct list {
    struct list_elem head;
    struct list_elem tail;
};

// 自定义函数类型function,用于在list_traversal中做回调函数
typedef bool (function)(struct list_elem*, int arg);

void list_init (struct list* li);
void list_insert_before(struct list_elem* before, struct list_elem* elem);
void list_push(struct list* plist, struct list_elem* elem);
void list_iterate(struct list* plist);
void list_append(struct list* plist, struct list_elem* elem);  
void list_remove(struct list_elem* pelem);
struct list_elem* list_pop(struct list* plist);
bool list_empty(struct list* plist);
uint32_t list_len(struct list* plist);
struct list_elem* list_traversal(struct list* plist, function func, int arg);
bool elem_find(struct list* plist, struct list_elem* obj_elem);

#endif // __LIB_KERNEL_LIST_H
