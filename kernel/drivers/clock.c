#include "clock.h"
#include "io.h"
#include "isr.h"
#include "pic.h"

#define PIT_CMD  0x43
#define PIT_CH0  0x40
#define PIT_BASE 1193182u

static volatile uint32_t ticks;

static void timer_handler(registers_t *regs)
{
    (void)regs;
    ticks++;
}

void clock_init(void)
{
    uint32_t divisor = PIT_BASE / CLOCK_HZ;
    outb(PIT_CMD, 0x36);                          
    outb(PIT_CH0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CH0, (uint8_t)((divisor >> 8) & 0xFF));
    irq_register_handler(0, timer_handler);
    pic_unmask(0);
}

uint32_t clock_ticks(void)          { return ticks; }
uint32_t clock_uptime_seconds(void) { return ticks / CLOCK_HZ; }

void clock_sleep_ms(uint32_t ms)
{
    uint32_t target = ticks + (ms * CLOCK_HZ + 999) / 1000;
    while ((int32_t)(ticks - target) < 0)
        __asm__ volatile ("hlt");
}
