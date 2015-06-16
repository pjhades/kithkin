#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <stdarg.h>

int sprintk(char *str, const char *fmt, va_list va);
int printk(const char *fmt, ...);

#define die(fmt, ...) \
    do { \
        printk((fmt), ##__VA_ARGS__); \
        while (1); \
    } while (0)

#define p2roundmask(x, y) ((typeof((x)))((y) - 1))
#define p2roundup(x, y)   ((((x) - 1) | p2roundmask(x, y)) + 1)
#define p2rounddown(x, y) ((x) & ~p2roundmask(x, y))

#endif
