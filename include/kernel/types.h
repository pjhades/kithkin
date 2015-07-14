#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned char      u8;
typedef char               i8;
typedef unsigned short     u16;
typedef short              i16;
typedef unsigned int       u32;
typedef int                i32;
#if __WORDSIZE == 64
typedef unsigned long      u64;
typedef long               i64;
#else
typedef unsigned long long u64;
typedef long long          i64;
#endif

typedef u32 size_t;
typedef i32 ssize_t;
typedef i32 off_t;

#define NULL ((void *)0)

#endif
