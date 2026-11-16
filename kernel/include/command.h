#ifndef RUNEOS_COMMAND_H
#define RUNEOS_COMMAND_H

typedef struct {
    const char *name;
    const char *help;
    int (*run)(int argc, char **argv);
} command_t;

extern const command_t command_list[];
extern const int       command_count;

const command_t *command_find(const char *name);

#endif
