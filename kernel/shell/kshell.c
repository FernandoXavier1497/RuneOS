#include "shell.h"
#include "tty.h"
#include "kstdio.h"
#include "vga.h"

#define LINE_MAX 128

void kshell_run(void)
{
    char line[LINE_MAX];

    for (;;) {
        vga_set_color(VGA_LIGHT_MAGENTA, VGA_BLACK);
        kputs("runeos");
        vga_set_color(VGA_DARK_GREY, VGA_BLACK);
        kputs("> ");
        vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);

        tty_readline(line, sizeof(line));
        interpreter_run(line);
    }
}
