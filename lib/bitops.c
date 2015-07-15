#include <bitops.h>

u32 next_pow2_32bit(u32 x)
{
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    return x;
}

u32 log2(u32 x)
{
    u32 r = 0;

    while (x >>= 1)
        r++;
    return r;
}
