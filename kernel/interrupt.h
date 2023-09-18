#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H

#include "global.h"

typedef void* intr_handler;

void idt_init(void);

/*
    中断状态
        INTR_OFF值为0时，表示关中断
        INTR_ON值为1时，表示开中断
*/
enum intr_status {
    INTR_OFF,
    INTR_ON
};

enum intr_status intr_get_status(void);
enum intr_status intr_set_status(enum intr_status);
enum intr_status intr_enable(void);
enum intr_status intr_disable(void);

#endif // __KERNEL_INTERRUPT_H