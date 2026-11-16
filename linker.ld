/* RuneOS linker script
 * The bootloader loads the kernel to physical 0x10000 (segment 0x1000:0000),
 * so the kernel is linked at that address. */
ENTRY(_start)

SECTIONS
{
    . = 0x10000;

    .text : ALIGN(4)
    {
        *(.text*)
    }

    .rodata : ALIGN(4)
    {
        *(.rodata*)
    }

    .data : ALIGN(4)
    {
        *(.data*)
    }

    .bss : ALIGN(4)
    {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    }

    /DISCARD/ :
    {
        *(.note*)
        *(.comment*)
        *(.eh_frame*)
    }
}
