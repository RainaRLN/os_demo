#ifndef _LINUX_MEMBLOCK_H_
#define _LINUX_MEMBLOCK_H_

#include "linux/types.h"


// 内存区域属性
enum memblock_flags {
    MEMBLOCK_NONE = 0x0,  // No special request
    MEMBLOCK_HOTPLUG = 0x1,  // hotpluggable region
    MEMBLOCK_MIRROR = 0x2,  // mirrored region
    MEMBLOCK_NOMAP = 0x4,  // don't add to kernel direct mapping
};

// memblock_regin 代表一块内存区域
struct memblock_region {
    phys_addr_t base;
    phys_addr_t size;
    enum memblock_flags flags;
    int nid;
};

// 管理同类型的 memory regions
struct memblock_type {
    unsigned long cnt;
    unsigned long max;
    phys_addr_t total_size;
    struct memblock_region *regions;
    char *name;
};

#define MEMBLOCK_ALLOC_ANYWHERE (~(phys_addr_t)0)
#define MEMBLOCK_ALLOC_ACCESSIBLE 0

// memblock 分配器的 metadata
struct memblock {
    bool bottom_up;  // 1 -> 从较低地址先分配; 2 -> 从较高地址先分配。默认为 0
    phys_addr_t current_limit;  // 地址上限
    struct memblock_type memory;  // 系统中可用的内存区域
    struct memblock_type reserved;  // 不可用的内存区域
};

int memblock_add(phys_addr_t base, phys_addr_t size);
int memblock_reserve(phys_addr_t base, phys_addr_t size);

void memblock_dump_all(void);

#define for_each_mem_range(i, type_a, type_b, nid, flags, \
                        p_start, p_end, p_nid) \
    for (i = 0, __next_mem_range(&i, nid, flags, type_a, type_b, \
                    p_start, p_end, p_nid); \
         i != (u64)ULLONG_MAX; \
         __next_mem_range(&i, nid, flags, type_a, type_b, \
                    p_start, p_end, p_nid))

#define for_each_mem_range_rev(i, type_a, type_b, nid, flags, \
                        p_start, p_end, p_nid) \
    for (i = (u64)ULLONG_MAX, \
        __next_mem_range_rev(&i, nid, flags, type_a, type_b, \
                    p_start, p_end, p_nid); \
        i != (u64)ULLONG_MAX; \
        __next_mem_range_rev(&i, nid, flags, type_a, type_b, \
                    p_start, p_end, p_nid))

#define for_each_free_mem_range(i, nid, flags, p_start, p_end, p_nid) \
    for_each_mem_range(i, &memblock.memory, &memblock.reserved, \
                    nid, flags, p_start, p_end, p_nid)

#define for_each_free_mem_range_reverse(i, nid, flags, p_start, p_end, p_nid) \
    for_each_mem_range_rev(i, &memblock.memory, &memblock.reserved, \
                        nid, flags, p_start, p_end, p_nid)


phys_addr_t memblock_phys_alloc_range(phys_addr_t size, phys_addr_t align,
                        phys_addr_t start, phys_addr_t end);

int memblock_free(phys_addr_t base, phys_addr_t size);

#endif  // _LINUX_MEMBLOCK_H_

