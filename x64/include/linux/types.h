#ifndef _LINUX_TYPES_H_
#define _LINUX_TYPES_H_

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef _Bool			bool;
enum {
    false = 0,
    true = 1
};

typedef unsigned int size_t;

typedef u64 phys_addr_t;

#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))

#endif  // _LINUX_TYPES_H_

