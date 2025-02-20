#ifndef _LINUX_TYPES_H_
#define _LINUX_TYPES_H_

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef long long int64;

typedef unsigned int size_t;

typedef enum {
    false = 0,
    true = 1
} bool;

#define NULL ((void *)0)

typedef u64 phys_addr_t;

#define PAGE_SIZE 4096

#define min_t(type, x, y) ({ \
    type __min1 = (x); \
    type __min2 = (y); \
    __min1 < __min2 ? __min1 : __min2; })

#define max_t(type, x, y) ({ \
    type __max1 = (x); \
    type __max2 = (y); \
    __max1 > __max2 ? __max1 : __max2; })

#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))

#define clamp(val, lo, hi) min((typeof(val))max(val, lo), hi)


#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))

#endif  // _LINUX_TYPES_H_

