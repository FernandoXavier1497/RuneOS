#include "isr.h"
#include "pic.h"
#include "kstdio.h"
#include "vga.h"

static isr_handler_t handlers[256];

static const char *const exception_names[32] = {
    "Divide error", "Debug", "Non-maskable interrupt", "Breakpoint",
    "Overflow", "Bound range exceeded", "Invalid opcode", "Device not available",
    "Double fault", "Coprocessor segment overrun", "Invalid TSS", "Segment not present",
    "Stack-segment fault", "General protection fault", "Page fault", "Reserved",
    "x87 floating-point", "Alignment check", "Machine check", "SIMD floating-point",
    "Virtualization", "Control protection", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Security exception", "Reserved"
};

void isr_register_handler(uint8_t vector, isr_handler_t handler)
{
    handlers[vector] = handler;
}

void irq_register_handler(uint8_t irq, isr_handler_t handler)
{
    handlers[32 + irq] = handler;
}

void interrupt_dispatch(registers_t *regs)
{
    uint32_t n = regs->int_no;

    if (handlers[n])
        handlers[n](regs);
    else if (n < 32) {
        vga_set_color(VGA_WHITE, VGA_RED);
        kprintf("\n*** RuneOS kernel panic: %s (#%u) ***\n", exception_names[n], n);
        kprintf("err=%x eip=%x cs=%x eflags=%x\n", regs->err_code, regs->eip, regs->cs, regs->eflags);
        kputs("System halted.\n");
        for (;;)
            __asm__ volatile ("cli; hlt");
    }

    if (n >= 32 && n < 48)
        pic_send_eoi((uint8_t)(n - 32));
}
