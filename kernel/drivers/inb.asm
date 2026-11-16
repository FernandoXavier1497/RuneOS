[bits 32]
section .text
global inb
inb:
    mov edx, [esp + 4]
    xor eax, eax
    in al, dx
    ret
