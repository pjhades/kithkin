#ifndef __COMMON_H__
#define __COMMON_H__

typedef unsigned char      uint8_t;
typedef char               int8_t;
typedef unsigned short     uint16_t;
typedef short              int16_t;
typedef unsigned int       uint32_t;
typedef int                int32_t;
#if __WORDSIZE == 64
typedef unsigned long      uint64_t;
typedef long               int64_t;
#else
typedef unsigned long long uint64_t;
typedef long long          int64_t;
#endif

#define MEM_E820_MAX 128

#define PTR2LBA(ptr) (ptr >> 9)

#endif
