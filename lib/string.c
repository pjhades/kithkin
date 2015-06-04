#include <string.h>

void *memcpy(void *dst, const void *src, size_t n)
{
    int i;
    unsigned char *dp = dst;
    const unsigned char *sp = src;
    for (i = 0; i < n; i++)
        *dp++ = *sp++;
    return dst;
}

void *memset(void *s, int c, size_t n)
{
    int i;
    unsigned char *p;
    for (p = s, i = 0; i < n; i++)
        *p++ = c;
    return s;
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

char *strncpy(char *dest, const char *src, size_t n)
{
    int i;

    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';

    return dest;
}
