//asm (".code16gcc\n");

#include <arch/x86.h>

inline uint8_t inb(uint16_t port)
{
    uint8_t value;

    asm volatile (
            "inb %w1, %0\n\t"
            :"=a"(value) :"d"(port) :
            );
    return value;
}

inline void outb(uint16_t port, uint8_t value)
{
    asm volatile (
            "outb %0, %w1\n\t"
            : :"a"(value),"d"(port) :
            );
}
