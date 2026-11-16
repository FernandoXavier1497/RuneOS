#ifndef RUNEOS_KEYBOARD_H
#define RUNEOS_KEYBOARD_H


#define KEY_UP    0x11
#define KEY_DOWN  0x12
#define KEY_LEFT  0x13
#define KEY_RIGHT 0x14

void keyboard_init(void);
char keyboard_getchar(void);

#endif
