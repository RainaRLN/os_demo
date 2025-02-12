#include "linux/kernel.h"
#include "linux/limits.h"
#include "linux/memblock.h"
#include "linux/numa.h"
#include "string.h"

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

// 调整 *size 大小，防止 base + *size 超过 PHYS_ADDR_MAX, 并返回调整后的大小
static inline phys_addr_t memblock_cap_size(phys_addr_t base, phys_addr_t *size)
{
    return *size = min(*size, PHYS_ADDR_MAX - base);
}

static void memblock_merge_regions(struct memblock_type *type)
{
    int i = 0;
    while (i < type->cnt - 1) {
        struct memblock_region *this = &type->regions[i];
        struct memblock_region *next = &type->regions[i + 1];

        if (this->base + this->size != next->base ||
                this->nid != next->nid ||
                this->flags != next->flags) {
            BUG_ON(this->base + this->size > next->base);
            i++;
            continue;
        }
        this->size += next->size;
        memmove(next, next + 1, (type->cnt - (i + 2)) * sizeof(*next));
        type->cnt--;
    }

    return;
}

static void memblock_insert_region(struct memblock_type *type,
            int idx, phys_addr_t base,
            phys_addr_t size,
            int nid,
            enum memblock_flags flags)
{
    struct memblock_region *rgn = &type->regions[idx];
    BUG_ON(type->cnt >= type->max);

    // 如果是插到最后，则 idx = type->cnt，不需要移动
    memmove(rgn + 1, rgn, (type->cnt - idx) * sizeof(*rgn));
    rgn->base = base;
    rgn->size = size;
    rgn->flags = flags;
    rgn->nid = nid;
    type->cnt++;
    type->total_size += size;

    return;
}

int memblock_add_range(struct memblock_type *type,
        phys_addr_t base, phys_addr_t size,
        int nid, enum memblock_flags flags)
{
    bool insert = false;
    phys_addr_t obase = base;
    phys_addr_t end = base + memblock_cap_size(base, &size);
    int idx, nr_new;
    struct memblock_region *rgn;

    if (!size)
        return 0;

    // 当前 regions 数组是空的
    if (type->regions[0].size == 0) {
        WARN_ON(type->cnt != 1 || type->total_size != 0);
        // 直接插入
        type->regions[0].base = base;
        type->regions[0].size = size;
        type->regions[0].flags = flags;
        type->regions[0].nid = nid;
        type->total_size = size;
        return 0;
    }

repeat:
    /*
     * 下面代码会执行两此，第一次计算需要的 regions 数量
     * 第二次做实际的插入操作
     */
    base = obase;
    nr_new = 0;

    // 找插在哪个 region 前面
    for (idx = 0, rgn = &type->regions[0]; idx < type->cnt; idx++, rgn = &type->regions[idx]) {
        phys_addr_t rbase = rgn->base;
        phys_addr_t rend = rbase + rgn->size;

        /* 不重叠情况
         * region 的范围是 [base, end) 所以 rbase = end 和
         * rend = base 也是不重叠的
         */
        // 要插入的 region 在这块 region 前面
        if (rbase >= end)
            break;
        // 要插入的 region 在这块 region 后面
        if (rend <= base)
            continue;

        // 要插入的 region 在这块 region 前面，且有重叠
        if (rbase > base) {
            WARN_ON(nid != rgn->nid);  // 不是同 NUMA 节点警告
            WARN_ON(flags != rgn->flags);  // 不是同内存区域类型警告
            nr_new++;
            // 把前面不重叠的部分插入
            if (insert)
                memblock_insert_region(type, idx++, base,
                                        rbase - base, nid, flags);
        }
        // 去除重叠部分，取后面余下部分
        base = min(rend, end);
    }

    if (base < end) {
        nr_new++;
        if (insert) {
            memblock_insert_region(type, idx, base, end - base,
                            nid, flags);
        }
    }

    // 不需要增加新 region
    if (!nr_new)
        return 0;

    if (!insert) {  // 计算完 cnt, 下次开始插入

        // TODO: 128 够用了，暂时不会走到这
#if 0
        while (type->cnt + nr_new > type->max) {  // 大小超过了已分配的 regions 数组个数
            // resize regions 数组大小 (double it)
            if (memblock_double_array(type, obase, size) < 0) {
                return -ENOMEM;
            }
        }
#endif

        // 标记 insert，重复上面流程，做真正的插入操作
        insert = true;
        goto repeat;
    } else {  // 插入结束，合并 regions
        // 遍历 memblock，合并可合并的 regions
        memblock_merge_regions(type);
        return 0;
    }
}

int memblock_add(phys_addr_t base, phys_addr_t size)
{
    phys_addr_t end = base + size - 1;
    printk("memblock_add: [0x%16x-0x%16x] size 0x%16x\n", base, end, size);

    return memblock_add_range(&memblock.memory, base, size, MAX_NUMNODES, MEMBLOCK_NONE);
}

static void memblock_dump(struct memblock_type *type)
{
    phys_addr_t base, end, size;
    enum memblock_flags flags;
    int idx;
    int nid;
    struct memblock_region *rgn;

    printk(" %s.cnt = 0x%x\n", type->name, type->cnt);

    for (idx = 0, rgn = &type->regions[0]; idx < type->cnt; idx++, rgn = &type->regions[idx]) {

        base = rgn->base;
        size = rgn->size;
        end = base + size - 1;
        flags = rgn->flags;
        nid = rgn->nid == MAX_NUMNODES ? 0 : rgn->nid;

        if (size == 0)
            continue;

        printk(" %s[0x%x]", type->name, idx);
        printk(" [0x%x-0x%x], 0x%x bytes", base, end, size);
        printk(" on node %d flags: 0x%x\n", nid, flags);
    }

    return;
}

void memblock_dump_all(void)
{
    printk("MEMBLOCK configuration:\n");
    printk(" memory size = 0x%x reserved size = 0x%x\n",
        memblock.memory.total_size,
        memblock.reserved.total_size);

    memblock_dump(&memblock.memory);
    memblock_dump(&memblock.reserved);

    return;
}

