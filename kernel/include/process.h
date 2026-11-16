#ifndef RUNEOS_PROCESS_H
#define RUNEOS_PROCESS_H
#include "types.h"

#define PROC_MAX      16
#define PROC_NAME_MAX 16

typedef enum { PROC_FREE = 0, PROC_RUNNING, PROC_READY } proc_state_t;

typedef struct {
    uint32_t     pid;
    char         name[PROC_NAME_MAX];
    proc_state_t state;
    uint32_t     start_tick;
} process_t;

void             process_init(void);
int              process_spawn(const char *name);
int              process_kill(uint32_t pid);
const process_t *process_at(int index);

#endif
