#include "kstdio.h"
#include "vga.h"
#include "serial.h"
#include <stdarg.h>

void kputchar(char c)
{
    vga_putchar(c);


    if (c == '\b') {
        serial_putchar('\b'); serial_putchar(' '); serial_putchar('\b');
    } else {
        if (c == '\n') serial_putchar('\r');
        serial_putchar(c);
    }
}

void kputs(const char *s)
{
    while (*s) kputchar(*s++);
}

static int left_align;

static void put_unsigned(uint32_t v, uint32_t base, int upper, int width, char pad, int negative)
{
    char tmp[34];
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int n = 0;

    do { tmp[n++] = digits[v % base]; v /= base; } while (v);
    if (negative) tmp[n++] = '-';

    int len = n;
    if (!left_align)
        for (int i = len; i < width; i++) kputchar(pad);
    while (n) kputchar(tmp[--n]);
    if (left_align)
        for (int i = len; i < width; i++) kputchar(' ');
}

static void put_string(const char *s, int width)
{
    int len = 0;
    while (s[len]) len++;
    if (!left_align)
        for (int i = len; i < width; i++) kputchar(' ');
    kputs(s);
    if (left_align)
        for (int i = len; i < width; i++) kputchar(' ');
}

void kprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    for (; *fmt; fmt++) {
        if (*fmt != '%') { kputchar(*fmt); continue; }
        fmt++;

        char pad = ' ';
        int  width = 0;
        left_align = 0;
        if (*fmt == '-') { left_align = 1; fmt++; }
        if (*fmt == '0') { pad = '0'; fmt++; }
        while (*fmt >= '0' && *fmt <= '9') width = width * 10 + (*fmt++ - '0');

        switch (*fmt) {
        case 's': { const char *s = va_arg(ap, const char *); put_string(s ? s : "(null)", width); break; }
        case 'c': kputchar((char)va_arg(ap, int)); break;
        case 'd':
        case 'i': {
            int32_t v = va_arg(ap, int32_t);
            if (v < 0) put_unsigned((uint32_t)(-v), 10, 0, width, pad, 1);
            else       put_unsigned((uint32_t)v,    10, 0, width, pad, 0);
            break;
        }
        case 'u': put_unsigned(va_arg(ap, uint32_t), 10, 0, width, pad, 0); break;
        case 'x': put_unsigned(va_arg(ap, uint32_t), 16, 0, width, pad, 0); break;
        case 'X': put_unsigned(va_arg(ap, uint32_t), 16, 1, width, pad, 0); break;
        case 'p': kputs("0x"); put_unsigned(va_arg(ap, uint32_t), 16, 0, 8, '0', 0); break;
        case '%': kputchar('%'); break;
        case 0:   va_end(ap); return;
        default:  kputchar('%'); kputchar(*fmt); break;
        }
    }
    va_end(ap);
}
