#include <lib/string.h>

void *memcpy(void *dst, const void *src, size_t n)
{
    int i;
    unsigned char *dp = dst;
    const unsigned char *sp = src;
    for (i = 0; i < n; i++)
        *dp++ = *sp++;
    return dst;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s1 == *s2) {
        ++s1;
        ++s2;
    }
    return *((unsigned char *)s1) - *((unsigned char *)s2);
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    while (n-- && *s1 && *s1 == *s2) {
        ++s1;
        ++s2;
    }
    if (*s1 != *s2)
        return *((unsigned char *)s1) - *((unsigned char *)s2);
    return 0;
}
