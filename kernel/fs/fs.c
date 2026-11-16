#include "fs.h"
#include "kstring.h"


static fs_file_t files[FS_MAX_FILES];

fs_file_t *fs_find(const char *name)
{
    for (int i = 0; i < FS_MAX_FILES; i++)
        if (files[i].used && kstrcmp(files[i].name, name) == 0)
            return &files[i];
    return NULL;
}

fs_file_t *fs_create(const char *name)
{
    size_t len = kstrlen(name);
    if (len == 0 || len >= FS_NAME_MAX || fs_find(name))
        return NULL;

    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (!files[i].used) {
            kmemset(&files[i], 0, sizeof(files[i]));
            files[i].used = 1;
            kstrcpy(files[i].name, name);
            return &files[i];
        }
    }
    return NULL;
}

int fs_write(const char *name, const char *text, int append)
{
    fs_file_t *f = fs_find(name);
    if (!f) f = fs_create(name);
    if (!f) return -1;

    if (!append) f->size = 0;
    for (; *text && f->size < FS_FILE_MAX - 1; text++)
        f->data[f->size++] = *text;
    f->data[f->size] = 0;
    return 0;
}

int fs_remove(const char *name)
{
    fs_file_t *f = fs_find(name);
    if (!f) return -1;
    f->used = 0;
    return 0;
}

fs_file_t *fs_at(int index)
{
    for (int i = 0; i < FS_MAX_FILES; i++)
        if (files[i].used && index-- == 0)
            return &files[i];
    return NULL;
}

void fs_init(void)
{
    kmemset(files, 0, sizeof(files));
    fs_write("README.txt",
             "Welcome to RuneOS!\n"
             "Type 'help' to see the available commands.\n"
             "Files live in RAM and are lost on reboot.\n", 0);
}
