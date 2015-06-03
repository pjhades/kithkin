#ifndef __X86_H__
#define __X86_H__

#include <kernel/types.h>

#define CR0_PG 0x80000000

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t value);
void insl(uint16_t port, void *addr, int count);
void outsl(uint16_t port, void *addr, int count);

#endif
