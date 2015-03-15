#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <stdarg.h>

int sprintk(char *str, const char *fmt, va_list va);
int printk(const char *fmt, ...);

#endif
