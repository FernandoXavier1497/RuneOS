#ifndef RUNEOS_KSTRING_H
#define RUNEOS_KSTRING_H
#include "types.h"

size_t kstrlen(const char *s);
int    kstrcmp(const char *a, const char *b);
int    kstrncmp(const char *a, const char *b, size_t n);
char  *kstrcpy(char *dst, const char *src);
char  *kstrncpy(char *dst, const char *src, size_t n);
void  *kmemset(void *dst, int value, size_t n);
void  *kmemcpy(void *dst, const void *src, size_t n);
int    kmemcmp(const void *a, const void *b, size_t n);
int    katoi(const char *s);

#endif
