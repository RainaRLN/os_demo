#include "test/memblock.h"
#include "linux/types.h"
#include "linux/limits.h"
#include "linux/numa.h"
#include "linux/kernel.h"
#include "linux/memblock.h"

void test_memblock(void)
{
    phys_addr_t alloc[4] = {0};

    alloc[0] = memblock_phys_alloc_range(0x2000, PAGE_SIZE, 0, PHYS_ADDR_MAX);
    if (!alloc[0]) {
        printk("[%s:%d] memblock_alloc 0x2000 size failed!\n", __func__, __LINE__);
    } else {
        printk("alloced [0x%16x, 0x%16x]\n", alloc[0], alloc[0] + 0x2000 - 1);
    }

    alloc[1] = memblock_phys_alloc_range(0x1200, PAGE_SIZE, 0, PHYS_ADDR_MAX);
    if (!alloc[1]) {
        printk("[%s:%d] memblock_alloc 0x1200 size failed!\n", __func__, __LINE__);
    } else {
        printk("alloced [0x%16x, 0x%16x]\n", alloc[1], alloc[1] + 0x1200 - 1);
    }

    alloc[2] = memblock_phys_alloc_range(0x1000, PAGE_SIZE, 0, 0x1000);
    if (!alloc[2]) {
        printk("[%s:%d] memblock_alloc 0x1000 size failed!\n", __func__, __LINE__);
    } else {
        printk("alloced [0x%16x, 0x%16x]\n", alloc[2], alloc[2] + 0x1000 - 1);
    }

    alloc[3] = memblock_phys_alloc_range(0x1000, PAGE_SIZE, 0, 0x2000);
    if (!alloc[3]) {
        printk("[%s:%d] memblock_alloc 0x1000 size failed!\n", __func__, __LINE__);
    } else {
        printk("alloced [0x%16x, 0x%16x]\n", alloc[3], alloc[3] + 0x1000 - 1);
    }

    memblock_dump_all();

    memblock_free(alloc[1] + 0x200, 0x1000);
    memblock_free(alloc[3], 0x1000);
    memblock_dump_all();

    return;
}
