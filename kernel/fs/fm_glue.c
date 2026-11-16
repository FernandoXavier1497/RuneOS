
#include "fm.h"
#include "kstdio.h"
#include "vga.h"
#include "keyboard.h"

static void io_putc(char c) { kputchar(c); }

static int io_getkey(void) {
    char c;
    do { c = keyboard_getchar(); } while (c == 0);
    switch ((unsigned char)c) {
    case KEY_UP:    return FM_KEY_UP;
    case KEY_DOWN:  return FM_KEY_DOWN;
    case KEY_LEFT:  return FM_KEY_LEFT;
    case KEY_RIGHT: return FM_KEY_RIGHT;
    default:        return (unsigned char)c;
    }
}

static void io_clear(void) { vga_clear(); }

static void io_color(uint8_t fg, uint8_t bg) {
    vga_set_color((enum vga_color)fg, (enum vga_color)bg);
}

static const fm_io_t rune_io = { io_putc, io_getkey, io_clear, io_color };


void rune_fm_init(void) {
    static int done = 0;
    if (done) return;
    done = 1;
    fm_init(&rune_io);
}
