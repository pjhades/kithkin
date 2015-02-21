#ifndef __STRING_H__
#define __STRING_H__

#include <common.h>

void *memcpy(void *dst, const void *src, size_t n)
{
    int i;
    unsigned char *dp = dst;
    const unsigned char *sp = src;
    for (i = 0; i < n; i++)
        *dp++ = *sp++;
    return dst;
}

#endif
