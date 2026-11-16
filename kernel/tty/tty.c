#include "tty.h"
#include "keyboard.h"
#include "kstdio.h"

void tty_init(void)
{
    keyboard_init();
}

void tty_readline(char *buf, size_t size)
{
    size_t len = 0;

    for (;;) {
        char c = keyboard_getchar();

        if (c == '\n') {
            kputchar('\n');
            buf[len] = 0;
            return;
        }
        if (c == '\b') {
            if (len > 0) { len--; kputchar('\b'); }
            continue;
        }
        if (c == '\t') c = ' ';
        if (c >= ' ' && c <= '~' && len < size - 1) {
            buf[len++] = c;
            kputchar(c);
        }
    }
}
