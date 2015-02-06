#ifndef __X86_H__
#define __X86_H__

#include <include/common.h>

inline uint8_t inb(uint16_t port);
inline void outb(uint16_t port, uint8_t value);

#endif
