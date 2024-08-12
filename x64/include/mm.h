#ifndef _MM_H_
#define _MM_H_

typedef struct {
    unsigned int base_addr_low;  // low 32 bits of base address
    unsigned int base_addr_high;  // high 32 bits of base address
    unsigned int length_low;  // low 32 bits of length in bytes
    unsigned int length_high;  // high 32 bits of length in bytes
    unsigned int type;  // address type of this range
} ards_item_t;

void show_memory_map(void);
#endif  // _MM_H_

