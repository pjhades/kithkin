#include <kernel/types.h>

uint8_t inb(uint16_t port)
{
    uint8_t value;

    asm volatile (
            "inb %w1, %0\n\t"
            :"=a"(value) :"d"(port) :
            );
    return value;
}

void outb(uint16_t port, uint8_t value)
{
    asm volatile (
            "outb %0, %w1\n\t"
            : :"a"(value),"d"(port) :
            );
}

void insl(uint16_t port, void *addr, int count)
{
    asm volatile (
            "cld\n\t"
            "repne insl\n\t"
            :"=D"(addr), "=c"(count)
            :"d"(port), "0"(addr), "1"(count)
            :"memory", "cc"
            );
}

void outsl(uint16_t port, void *addr, int count)
{
    asm volatile (
            "cld\n\t"
            "repne outsl\n\t"
            :"=S"(addr), "=c"(count)
            :"d"(port), "0"(addr), "1"(count)
            :"cc"
            );
}
