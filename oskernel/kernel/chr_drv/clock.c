#include "../../include/linux/kernel.h"
#include "../../include/linux/traps.h"

#define PIT_CHAN0_REG 0X40
#define PIT_CHAN2_REG 0X42
#define PIT_CTRL_REG 0X43

#define HZ 100
#define OSCILLATOR 1193182
#define CLOCK_COUNTER (OSCILLATOR / HZ)

// 10ms触发一次中断
#define JIFFY (1000 / HZ)

int jiffy = JIFFY;
int cpu_tickes = 0;

void clock_init() {
    out_byte(PIT_CTRL_REG, 0b00110100);
    out_byte(PIT_CHAN0_REG, CLOCK_COUNTER & 0xff);
    out_byte(PIT_CHAN0_REG, (CLOCK_COUNTER >> 8) & 0xff);
}

void clock_handler(int idt_index) {
    send_eoi(idt_index);

    printk("0x%x\n", idt_index);
}