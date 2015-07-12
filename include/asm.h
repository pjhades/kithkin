#ifndef __ASM_H__
#define __ASM_H__

#include <kernel/types.h>

u8 inb(u16 port);
void outb(u16 port, u8 value);
void insl(u16 port, void *addr, int count);
void outsl(u16 port, void *addr, int count);

#endif
