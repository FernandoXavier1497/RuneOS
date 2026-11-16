#include "serial.h"
#include "io.h"

#define COM1 0x3F8

void serial_init(void)
{
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x03);
}

void serial_putchar(char c)
{
    for (int spin = 0; spin < 100000 && !(inb(COM1 + 5) & 0x20); spin++)
        ;
    outb(COM1, (uint8_t)c);
}
