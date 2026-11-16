#include "init.h"
#include "vga.h"
#include "serial.h"
#include "pic.h"
#include "idt.h"
#include "clock.h"
#include "tty.h"
#include "fs.h"
#include "process.h"
#include "kstdio.h"

void print_banner(void)
{
    vga_set_color(VGA_LIGHT_MAGENTA, VGA_BLACK);
    kputs(" ____                       ___  ____\n");
    kputs("|  _ \\ _   _ _ __   ___   / _ \\/ ___|\n");
    kputs("| |_) | | | | '_ \\ / _ \\ | | | \\___ \\\n");
    kputs("|  _ <| |_| | | | |  __/ | |_| |___) |\n");
    kputs("|_| \\_\\\\__,_|_| |_|\\___|  \\___/|____/\n");
    vga_set_color(VGA_DARK_GREY, VGA_BLACK);
    kprintf("  v%s - a tiny 32-bit operating system\n\n", RUNEOS_VERSION);
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
}

void init_system(void)
{
    serial_init();
    vga_init();

    pic_init();
    idt_init();
    clock_init();
    fs_init();
    process_init();
    tty_init();

    __asm__ volatile ("sti");

    print_banner();
    kputs("Type 'help' for a list of commands.\n\n");
}
