#include "shell.h"
#include "command.h"
#include "tokens.h"
#include "kstdio.h"

#define MAX_ARGS 16

int interpreter_run(char *line)
{
    char *argv[MAX_ARGS];
    int argc = tokenize(line, argv, MAX_ARGS);

    if (argc == 0)
        return 0;

    const command_t *cmd = command_find(argv[0]);
    if (!cmd) {
        kprintf("runeos: unknown command '%s' (try 'help')\n", argv[0]);
        return -1;
    }
    return cmd->run(argc, argv);
}
