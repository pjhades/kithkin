#ifndef __BITOPS_H__
#define __BITOPS_H__

#include <kernel/types.h>

#define test_bit(v, b) ((v) & (1 << (b)))
#define set_bit(v, b) (v) |= (1 << (b))
#define clear_bit(v, b) (v) &= ~(1 << (b))

u32 next_pow2_32bit(u32 x);

#endif
