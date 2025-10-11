#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

int strcmp(const char *str1, const char *str2);
char *strncpy(char *dest, const char *src, size_t n);

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);

#endif