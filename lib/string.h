#ifndef __STRING_H__
#define __STRING_H__

#include <common.h>

void *memcpy(void *dst, const void *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

#endif
