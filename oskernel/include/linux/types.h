#ifndef _OS_OSKERNEL_LINUX_TYPES_H
#define _OS_OSKERNEL_LINUX_TYPES_H


#define EOF = -1        // end of fiel

#define NULL ((void*)0)     // null pointer

#define EOS  '\0'           // end of string

#define bool _Bool
#define true 1
#define false 0

typedef unsigned int size_t;

typedef long long int64;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#endif // _OS_OSKERNEL_LINUX_TYPES_H