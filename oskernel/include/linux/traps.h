#ifndef _OS_OSKERNEL_TRAPS_H
#define _OS_OSKERNEL_TRAPS_H

#include "head.h"

void gdt_init();
void idt_init();

#endif // _OS_OSKERNEL_TRAPS_H