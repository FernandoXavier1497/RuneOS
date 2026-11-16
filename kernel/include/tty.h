#ifndef RUNEOS_TTY_H
#define RUNEOS_TTY_H
#include "types.h"

void tty_init(void);
void tty_readline(char *buf, size_t size);

#endif
