#ifndef __ASM_H__
#define __ASM_H__

#include <common.h>

inline uint8_t inb(uint16_t port);
inline void outb(uint16_t port, uint8_t value);

#endif
