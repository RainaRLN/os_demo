#ifndef _MM_H_
#define _MM_H_

#include "linux/types.h"
#include "bitmap.h"

typedef struct {
    unsigned int base_addr_low;  // low 32 bits of base address
    unsigned int base_addr_high;  // high 32 bits of base address
    unsigned int length_low;  // low 32 bits of length in bytes
    unsigned int length_high;  // high 32 bits of length in bytes
    unsigned int type;  // address type of this range
} ards_item_t;

typedef struct {
    uint addr_base;
    uint pages_total;
    bitmap_t bitmap;
    uchar * bitmap_buf;
} physics_memory_map_t;

void show_memory_map(void);
void phy_memory_init(void);
void *get_free_page(void);
void free_page(void *p);
#endif  // _MM_H_

