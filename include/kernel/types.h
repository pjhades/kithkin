#ifndef __TYPES_H__
#define __TYPES_H__

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

typedef uint64_t size_t;
typedef int64_t  ssize_t;
typedef int64_t  off_t;

#endif
