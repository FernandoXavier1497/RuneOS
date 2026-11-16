[org 0x7C00]
[bits 16]

%ifndef KERNEL_SECTORS
%define KERNEL_SECTORS 64
%endif

KERNEL_SEG  equ 0x1000          
KERNEL_ADDR equ 0x10000
STACK_TOP   equ 0x90000

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl

    mov si, msg_boot
    call print16

    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, [boot_drive]
    int 0x13
    jc disk_error
    cmp bx, 0xAA55
    jne disk_error

    
    mov si, dap
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    
    in al, 0x92
    or al, 2
    out 0x92, al

    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:pm_entry

disk_error:
    mov si, msg_disk
    call print16
.hang:
    hlt
    jmp .hang

print16:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp print16
.done:
    ret

boot_drive: db 0
msg_boot:   db "RuneOS: loading kernel...", 13, 10, 0
msg_disk:   db "RuneOS: disk read error!", 13, 10, 0

align 4
dap:
    db 0x10, 0
    dw KERNEL_SECTORS
    dw 0x0000
    dw KERNEL_SEG
    dq 1

align 8
gdt_start:
    dq 0x0000000000000000   
    dq 0x00CF9A000000FFFF      
    dq 0x00CF92000000FFFF     
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[bits 32]
pm_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, STACK_TOP
    mov eax, KERNEL_ADDR
    jmp eax

times 510 - ($ - $$) db 0
dw 0xAA55
