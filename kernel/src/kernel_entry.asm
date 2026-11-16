[bits 32]

section .text
global _start
extern kmain
extern __bss_start
extern __bss_end

_start:
    cli
    cld
    mov esp, 0x90000

    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb

    call kmain

.hang:
    cli
    hlt
    jmp .hang
