#include "tokens.h"

int tokenize(char *line, char **argv, int max_args)
{
    int argc = 0;

    while (*line && argc < max_args) {
        while (*line == ' ') line++;
        if (!*line) break;

        if (*line == '"') {
            argv[argc++] = ++line;
            while (*line && *line != '"') line++;
        } else {
            argv[argc++] = line;
            while (*line && *line != ' ') line++;
        }
        if (*line) *line++ = 0;
    }
    return argc;
}
