[bits 32]
section .text
global vga_putchar_at
vga_putchar_at:
    mov eax, [esp + 12]
    mov cl,  [esp + 4]
    mov ch,  [esp + 8]
    mov word [0xB8000 + eax * 2], cx
    ret
