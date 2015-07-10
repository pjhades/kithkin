#ifndef __X86_H__
#define __X86_H__

#include <kernel/types.h>

#define CR0_PG 0x80000000

u8 inb(u16 port);
void outb(u16 port, u8 value);
void insl(u16 port, void *addr, int count);
void outsl(u16 port, void *addr, int count);

#endif
