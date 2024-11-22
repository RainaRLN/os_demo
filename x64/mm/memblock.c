#include "linux/memblock.h"

#define INIT_MEMBLOCK_REGIONS 128
#define INIT_MEMBLOCK_RESERVED_REGIONS INIT_MEMBLOCK_REGIONS

static struct memblock_region memblock_memory_init_regions[INIT_MEMBLOCK_REGIONS];
static struct memblock_region memblock_reserved_init_regions[INIT_MEMBLOCK_RESERVED_REGIONS];

struct memblock memblock = {
    .memory.regions = memblock_memory_init_regions,
    .memory.cnt = 1,  // empty dummy entry
    .memory.max = INIT_MEMBLOCK_REGIONS,
    .memory.name = "memory",

    .reserved.regions = memblock_reserved_init_regions,
    .reserved.cnt = 1, // empty dummy entry
    .reserved.max = INIT_MEMBLOCK_RESERVED_REGIONS,
    .reserved.name = "reserved",

    .bottom_up = false,
    .current_limit = MEMBLOCK_ALLOC_ANYWHERE,
};

