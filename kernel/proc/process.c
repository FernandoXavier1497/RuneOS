#include "process.h"
#include "clock.h"
#include "kstring.h"


static process_t table[PROC_MAX];
static uint32_t  next_pid = 1;

int process_spawn(const char *name)
{
    for (int i = 0; i < PROC_MAX; i++) {
        if (table[i].state == PROC_FREE) {
            table[i].pid        = next_pid++;
            table[i].state      = PROC_READY;
            table[i].start_tick = clock_ticks();
            kstrncpy(table[i].name, name, PROC_NAME_MAX - 1);
            table[i].name[PROC_NAME_MAX - 1] = 0;
            return (int)table[i].pid;
        }
    }
    return -1;
}

int process_kill(uint32_t pid)
{
    if (pid <= 2) return -2;
    for (int i = 0; i < PROC_MAX; i++) {
        if (table[i].state != PROC_FREE && table[i].pid == pid) {
            table[i].state = PROC_FREE;
            return 0;
        }
    }
    return -1;
}

const process_t *process_at(int index)
{
    for (int i = 0; i < PROC_MAX; i++)
        if (table[i].state != PROC_FREE && index-- == 0)
            return &table[i];
    return NULL;
}

void process_init(void)
{
    kmemset(table, 0, sizeof(table));
    next_pid = 1;
    process_spawn("init");
    process_spawn("kshell");
    table[1].state = PROC_RUNNING;
}
