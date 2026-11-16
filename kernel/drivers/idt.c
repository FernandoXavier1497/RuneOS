#include "idt.h"

typedef struct __attribute__((packed)) {
    uint16_t base_lo;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  flags;
    uint16_t base_hi;
} idt_entry_t;

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} idt_ptr_t;

extern uint32_t isr_stub_table[32];
extern uint32_t irq_stub_table[16];   

static idt_entry_t idt[256];
static idt_ptr_t   idt_ptr;

void idt_set_gate(uint8_t vector, uint32_t handler, uint16_t selector, uint8_t flags)
{
    idt[vector].base_lo  = handler & 0xFFFF;
    idt[vector].base_hi  = (handler >> 16) & 0xFFFF;
    idt[vector].selector = selector;
    idt[vector].zero     = 0;
    idt[vector].flags    = flags;
}

void idt_init(void)
{
    for (int i = 0; i < 32; i++)
        idt_set_gate((uint8_t)i, isr_stub_table[i], 0x08, 0x8E);
    for (int i = 0; i < 16; i++)
        idt_set_gate((uint8_t)(32 + i), irq_stub_table[i], 0x08, 0x8E);

    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (uint32_t)&idt;
    __asm__ volatile ("lidt %0" : : "m"(idt_ptr));
}
