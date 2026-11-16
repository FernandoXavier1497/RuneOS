#include "init.h"
#include "shell.h"

void kmain(void)
{
    init_system();
    kshell_run();

    for (;;)
        __asm__ volatile ("hlt");
}
