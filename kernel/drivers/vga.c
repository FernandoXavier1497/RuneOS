#include "vga.h"
#include "io.h"

static uint32_t cursor_row, cursor_col;
static uint8_t  text_color = (VGA_BLACK << 4) | VGA_LIGHT_GREY;

static volatile uint16_t *const VGA_MEM = (volatile uint16_t *)0xB8000;

static void update_hw_cursor(void)
{
    uint16_t pos = (uint16_t)(cursor_row * VGA_COLS + cursor_col);
    outb(0x3D4, 14);
    outb(0x3D5, (uint8_t)(pos >> 8));
    outb(0x3D4, 15);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
}

static void scroll(void)
{
    for (uint32_t i = 0; i < (VGA_ROWS - 1) * VGA_COLS; i++)
        VGA_MEM[i] = VGA_MEM[i + VGA_COLS];
    for (uint32_t i = (VGA_ROWS - 1) * VGA_COLS; i < VGA_ROWS * VGA_COLS; i++)
        vga_putchar_at(' ', text_color, i);
    cursor_row = VGA_ROWS - 1;
}

void vga_clear(void)
{
    for (uint32_t i = 0; i < VGA_ROWS * VGA_COLS; i++)
        vga_putchar_at(' ', text_color, i);
    cursor_row = cursor_col = 0;
    update_hw_cursor();
}

void vga_init(void)
{
    
    outb(0x3D4, 0x0A); outb(0x3D5, 14);
    outb(0x3D4, 0x0B); outb(0x3D5, 15);
    vga_clear();
}

void vga_set_color(enum vga_color fg, enum vga_color bg)
{
    text_color = (uint8_t)((bg << 4) | fg);
}

void vga_putchar(char c)
{
    switch (c) {
    case '\n':
        cursor_col = 0;
        cursor_row++;
        break;
    case '\r':
        cursor_col = 0;
        break;
    case '\t':
        cursor_col = (cursor_col + 4) & ~3u;
        break;
    case '\b':
        if (cursor_col > 0) {
            cursor_col--;
        } else if (cursor_row > 0) {
            cursor_row--;
            cursor_col = VGA_COLS - 1;
        }
        vga_putchar_at(' ', text_color, cursor_row * VGA_COLS + cursor_col);
        break;
    default:
        vga_putchar_at(c, text_color, cursor_row * VGA_COLS + cursor_col);
        cursor_col++;
        break;
    }

    if (cursor_col >= VGA_COLS) {
        cursor_col = 0;
        cursor_row++;
    }
    if (cursor_row >= VGA_ROWS)
        scroll();
    update_hw_cursor();
}
