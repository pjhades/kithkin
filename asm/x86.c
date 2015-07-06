#include <kernel/types.h>

u8 inb(u16 port)
{
    u8 value;

    asm volatile (
            "inb %w1, %0\n\t"
            :"=a"(value) :"d"(port) :
            );
    return value;
}

void outb(u16 port, u8 value)
{
    asm volatile (
            "outb %0, %w1\n\t"
            : :"a"(value),"d"(port) :
            );
}

void insl(u16 port, void *addr, int count)
{
    asm volatile (
            "cld\n\t"
            "repne insl\n\t"
            :"=D"(addr), "=c"(count)
            :"d"(port), "0"(addr), "1"(count)
            :"memory", "cc"
            );
}

void outsl(u16 port, void *addr, int count)
{
    asm volatile (
            "cld\n\t"
            "repne outsl\n\t"
            :"=S"(addr), "=c"(count)
            :"d"(port), "0"(addr), "1"(count)
            :"cc"
            );
}
