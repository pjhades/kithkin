#ifndef __STRING_H__
#define __STRING_H__

void *memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *dp = dst;
    const unsigned char *sp = src;
    for (; n >= 0; n--)
        *dp++ = *sp++;
    return dst;
}

#endif
