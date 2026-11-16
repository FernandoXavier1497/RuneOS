#include "keyboard.h"
#include "io.h"
#include "isr.h"
#include "pic.h"


static const char map_normal[0x40] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
    0, '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0, ' ', 0
};

static const char map_shift[0x40] = {
    0, 27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0, 'A','S','D','F','G','H','J','K','L',':','"','~',
    0, '|','Z','X','C','V','B','N','M','<','>','?', 0,
    '*', 0, ' ', 0
};

#define BUF_SIZE 128
static volatile char     buffer[BUF_SIZE];
static volatile uint32_t head, tail;
static int shift, caps, extended;

static void push(char c)
{
    uint32_t next = (head + 1) % BUF_SIZE;
    if (next != tail) {
        buffer[head] = c;
        head = next;
    }
}

static void keyboard_handler(registers_t *regs)
{
    (void)regs;
    uint8_t sc = inb(0x60);

    if (sc == 0xE0) { extended = 1; return; }
    if (extended) {
        extended = 0;
        if (!(sc & 0x80)) {
            switch (sc) {
            case 0x48: push(KEY_UP);    break;
            case 0x50: push(KEY_DOWN);  break;
            case 0x4B: push(KEY_LEFT);  break;
            case 0x4D: push(KEY_RIGHT); break;
            default: break;
            }
        }
        return;
    }

    int released = sc & 0x80;
    sc &= 0x7F;

    if (sc == 0x2A || sc == 0x36) { shift = !released; return; }
    if (released) return;
    if (sc == 0x3A) { caps = !caps; return; }
    if (sc >= 0x40) return;

    char c = shift ? map_shift[sc] : map_normal[sc];
    if (caps && c >= 'a' && c <= 'z') c = (char)(c - 32);
    else if (caps && c >= 'A' && c <= 'Z' && shift) c = (char)(c + 32);
    if (c) push(c);
}

void keyboard_init(void)
{
    irq_register_handler(1, keyboard_handler);
    pic_unmask(1);
}

char keyboard_getchar(void)
{
    while (head == tail)
        __asm__ volatile ("hlt");
    char c = buffer[tail];
    tail = (tail + 1) % BUF_SIZE;
    return c;
}
