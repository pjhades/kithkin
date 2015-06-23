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

#define assert(cond) \
    do { \
        if (!(cond)) \
            die("assertion failed: %s in %s:%d\n", #cond, __FILE__, __LINE__); \
    } while (0)



#define p2roundmask(x, y) ((typeof((x)))((y) - 1))
#define p2roundup(x, y)   ((((x) - 1) | p2roundmask(x, y)) + 1)
#define p2rounddown(x, y) ((x) & ~p2roundmask(x, y))

#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))

#endif
