#ifndef _OS_OSKERNEL_TRAPS_H
#define _OS_OSKERNEL_TRAPS_H

#include "head.h"

void gdt_init();
void idt_init();

void send_eoi(int idt_index);

void write_xdt_ptr(xdt_ptr_t* p, short limit, int base);

#endif // _OS_OSKERNEL_TRAPS_H