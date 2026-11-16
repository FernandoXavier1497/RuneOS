#ifndef RUNEOS_IDT_H
#define RUNEOS_IDT_H
#include "types.h"

void idt_init(void);
void idt_set_gate(uint8_t vector, uint32_t handler, uint16_t selector, uint8_t flags);

#endif
