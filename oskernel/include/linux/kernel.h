#ifndef _OS_OSKERNEL_KERNEL_H
#define _OS_OSKERNEL_KERNEL_H

#include "../stdarg.h"

int vsprintf(char* buf, const char* fmt, va_list args);
int printk(const char* fmt, ...);

#endif // _OS_OSKERNEL_KERNEL_H