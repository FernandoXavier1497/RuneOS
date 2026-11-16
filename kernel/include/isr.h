#ifndef RUNEOS_ISR_H
#define RUNEOS_ISR_H
#include "types.h"

typedef struct {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags;
} registers_t;

typedef void (*isr_handler_t)(registers_t *regs);

void isr_register_handler(uint8_t vector, isr_handler_t handler);
void irq_register_handler(uint8_t irq, isr_handler_t handler);

#endif
