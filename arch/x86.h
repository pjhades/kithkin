#ifndef __X86_H__
#define __X86_H__

#include <common.h>

inline uint8_t inb(uint16_t port);
inline void outb(uint16_t port, uint8_t value);
inline void insl(uint16_t port, void *addr, int count);
inline void outsl(uint16_t port, void *addr, int count);

#endif
