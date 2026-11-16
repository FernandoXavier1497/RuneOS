#include "command.h"
#include "kstring.h"
#include "types.h"

const command_t *command_find(const char *name)
{
    for (int i = 0; i < command_count; i++)
        if (kstrcmp(command_list[i].name, name) == 0)
            return &command_list[i];
    return NULL;
}
