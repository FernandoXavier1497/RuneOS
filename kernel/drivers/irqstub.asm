[bits 32]
section .text

global irq_stub_table
extern interrupt_common

%assign i 0
%rep 16
irq %+ i:
    push dword 0
    push dword (32 + i)
    jmp interrupt_common
%assign i i+1
%endrep

section .data
irq_stub_table:
%assign i 0
%rep 16
    dd irq %+ i
%assign i i+1
%endrep
