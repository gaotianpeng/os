#ifndef _OS_OSKERNEL_TTY_H
#define _OS_OSKERNEL_TTY_H

#include "types.h"

void console_init(void);

void console_write(char* buf, u32 count);


#endif // _OS_OSKERNEL_TTY_H