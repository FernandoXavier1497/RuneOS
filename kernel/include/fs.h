#ifndef RUNEOS_FS_H
#define RUNEOS_FS_H
#include "types.h"

#define FS_MAX_FILES 16
#define FS_NAME_MAX  24
#define FS_FILE_MAX  512

typedef struct {
    int      used;
    char     name[FS_NAME_MAX];
    char     data[FS_FILE_MAX];
    uint32_t size;
} fs_file_t;

void       fs_init(void);
fs_file_t *fs_find(const char *name);
fs_file_t *fs_create(const char *name);
int        fs_write(const char *name, const char *text, int append);
int        fs_remove(const char *name);
fs_file_t *fs_at(int index);

#endif
