[bits 32]
section .text

global interrupt_common
global isr_stub_table
extern interrupt_dispatch

%assign i 0
%rep 32
isr %+ i:
  %if (i == 8) || (i >= 10 && i <= 14) || (i == 17) || (i == 30)
    push dword i
  %else
    push dword 0 
    push dword i
  %endif
    jmp interrupt_common
%assign i i+1
%endrep

interrupt_common:
    cld
    pusha
    push esp
    call interrupt_dispatch
    add esp, 4
    popa
    add esp, 8
    iret

section .data
isr_stub_table:
%assign i 0
%rep 32
    dd isr %+ i
%assign i i+1
%endrep
