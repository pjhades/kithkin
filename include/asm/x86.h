#ifndef __X86_H__
#define __X86_H__

#include <kernel/types.h>

#define CR0_PG 0x80000000

inline uint8_t inb(uint16_t port);
inline void outb(uint16_t port, uint8_t value);
inline void insl(uint16_t port, void *addr, int count);
inline void outsl(uint16_t port, void *addr, int count);

#endif
