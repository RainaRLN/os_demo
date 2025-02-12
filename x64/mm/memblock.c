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

void __next_mem_range(u64 *idx, int nid,
                enum memblock_flags flags,
                struct memblock_type *type_a,
                struct memblock_type *type_b,
                phys_addr_t *out_start,
                phys_addr_t *out_end, int *out_nid)
{
    int idx_a = *idx & 0xffffffff;
    int idx_b = *idx >> 32;

    if (WARN_ONCE(nid == MAX_NUMNODES,
            "Usage of MAX_NUMNODES is deprecated. Use NUMA_NO_NODE instead\n")) {
        nid = NUMA_NO_NODE;
    }

    for (; idx_a < type_a->cnt; idx_a++) {
        struct memblock_region *m = &type_a->regions[idx_a];

        phys_addr_t m_start = m->base;
        phys_addr_t m_end = m->base + m->size;
        int m_nid = m->nid;

        // if (should_skip_region(m, nid, flags))
        //     continue;

        if (!type_b) {
            if (out_start)
                *out_start = m_start;
            if (out_end)
                *out_end = m_end;
            if (out_nid)
                *out_nid = m_nid;
            idx_a++;
            *idx = (u32)idx_a | (u64)idx_b << 32;
            return;
        }

        for (; idx_b < type_b->cnt + 1; idx_b++) {
            struct memblock_region *r;
            phys_addr_t r_start;
            phys_addr_t r_end;

            r = &type_b->regions[idx_b];
            r_start = idx_b ? r[-1].base + r[-1].size : 0;
            r_end = idx_b < type_b->cnt ? r->base : PHYS_ADDR_MAX;

            if (r_start >= m_end)
                break;

            if (m_start < r_end) {
                if (out_start)
                    *out_start = max(m_start, r_start);
                if (out_end)
                    *out_end = min(m_end, r_end);
                if (out_nid)
                    *out_nid = m_nid;
                if (m_end <= r_end)
                    idx_a++;
                else
                    idx_b++;
                *idx = (u32)idx_a | (u64)idx_b << 32;
                return;
            }
        }
    }

    *idx = ULLONG_MAX;
    return;
}

// Finds the next range from type_a which is not marked as unsuitable in type_b
void __next_mem_range_rev(u64 *idx, int nid,
                enum memblock_flags flags,
                struct memblock_type *type_a,
                struct memblock_type *type_b,
                phys_addr_t *out_start,
                phys_addr_t *out_end, int *out_nid)
{
    int idx_a = *idx & 0xffffffff;  // 低 32 bit 记录 type_a 的 idx
    int idx_b = *idx >> 32;  // 高 32 bit 记录 type_b 的 idx

    if (WARN_ONCE(nid == MAX_NUMNODES,
            "Usage of MAX_NUMNODES is deprecated, Use NUMA_NO_NODE instead\n")) {
        nid = NUMA_NO_NODE;
    }

    if (*idx == (u64)ULLONG_MAX) {
        idx_a = type_a->cnt - 1;
        if (type_b != NULL)
            idx_b = type_b->cnt;
        else
            idx_b = 0;
    }

    for (; idx_a >= 0; idx_a--) {
        struct memblock_region *m = &type_a->regions[idx_a];

        phys_addr_t m_start = m->base;
        phys_addr_t m_end = m->base + m->size;
        int m_nid = m->nid;

        // if (should_skip_region(m, nid, flags))
        //     continue;

        if (!type_b) {
            if (out_start)
                *out_start = m_start;
            if (out_end)
                *out_end = m_end;
            if (out_nid)
                *out_nid = m_nid;
            idx_a--;
            *idx = (u32)idx_a | (u64)idx_b << 32;
            return;
        }

        /* 取 type_b 的补集 r (去除 type_b 的可用空间)
         * 比如 type_b 如下:
         * [0-16), [32-48), [128-130)
         * 遍历的 r 为
         * [0-0), [16-32), [48-128), [130-MAX)
         */
        for (; idx_b >= 0; idx_b--) {
            struct memblock_region *r;
            phys_addr_t r_start;
            phys_addr_t r_end;

            r = &type_b->regions[idx_b];
            // r_start = 上一个 region 的 end
            r_start = idx_b ? r[-1].base + r[-1].size : 0;
            // r_end = 当前 region 的 start
            r_end = idx_b < type_b->cnt ? r->base : PHYS_ADDR_MAX;

            // 可用空间在 m_start 前面，break， 取上一个 m
            if (r_end <= m_start) {
                break;
            }
            // r 和 m 有交集，取交集
            if (m_end > r_start) {
                if (out_start)
                    *out_start = max(m_start, r_start);
                if (out_end)
                    *out_end = min(m_end, r_end);
                if (out_nid)
                    *out_nid = m_nid;
                if (m_start >= r_start)
                    idx_a--;  // 下次遍历上一个 m
                else
                    idx_b--;  // 下次从上一个 r 开始遍历
                // 记录 m 和 r 的 idx，供下一轮遍历使用
                *idx = (u32)idx_a | (u64)idx_b << 32;
                return;
            }
        }
    }

    // 遍历结束标志
    *idx = ULLONG_MAX;
    return;
}

static phys_addr_t __memblock_find_range_bottom_up(phys_addr_t start, phys_addr_t end,
                        phys_addr_t size, phys_addr_t align, int nid,
                        enum memblock_flags flags)
{
    phys_addr_t this_start, this_end, cand;
    u64 i;

    for_each_free_mem_range(i, nid, flags, &this_start, &this_end, NULL) {
        this_start = clamp(this_start, start, end);
        this_end = clamp(this_end, start, end);

        // 头部分配，起始地址按 align 向高地址对齐, 返回起始地址
        cand = round_up(this_start, align);
        if (cand < this_end && this_end - cand >= size) {
            return cand;
        }
    }

}

static phys_addr_t __memblock_find_range_top_down(phys_addr_t start, phys_addr_t end,
                        phys_addr_t size, phys_addr_t align, int nid,
                        enum memblock_flags flags)
{
    phys_addr_t this_start, this_end, cand;
    u64 i;

/*
 *  for (i = (u64)ULLONG_MAX, __next_mem_range_rev(&i, nid, flags, &memblock.memory,
 *              &memblock.reserved, &this_start, &this_end, NULL);
 *          i != (u64)ULLONG_MAX;
 *          __next_mem_range_rev(&i, nid, flags, &memblock.memory, &memblock.reserved,
 *              &this_start, &this_end, NULL)) {
 */
    for_each_free_mem_range_reverse(i, nid, flags, &this_start, &this_end, NULL) {
        // 取 [this_start, this_end] [start, end] 的交集
        // 若无交集, 收缩为 start 或 end
        this_start = clamp(this_start, start, end);
        this_end = clamp(this_end, start, end);

        // 避免 round_down 出错, 保证 this_end - size 非负
        if (this_end < size) {
            continue;
        }

        // 尾部分配，起始地址按 align 向低地址对齐, 返回起始地址
        cand = round_down(this_end - size, align);
        // 满足条件，返回
        if (cand >= this_start) {
            return cand;
        }
    }

    // 分配失败
    return 0;
}

static phys_addr_t memblock_find_in_range_node(phys_addr_t size,
                    phys_addr_t align, phys_addr_t start,
                    phys_addr_t end, int nid,
                    enum memblock_flags flags)
{
    if (end == MEMBLOCK_ALLOC_ACCESSIBLE) {
        end = memblock.current_limit;
    }

    // 避免分配第一个页
    // 0 地址用来表示分配失败
    start = max_t(phys_addr_t, start, PAGE_SIZE);
    end = max(start, end);

    if (memblock.bottom_up) {
        return __memblock_find_range_bottom_up(start, end, size, align,
                                nid, flags);
    } else {
        return __memblock_find_range_top_down(start, end, size, align,
                                nid, flags);
    }
}

phys_addr_t memblock_alloc_range_nid(phys_addr_t size,
                    phys_addr_t align, phys_addr_t start,
                    phys_addr_t end, int nid)
{
    enum memblock_flags flags = MEMBLOCK_NONE;
    phys_addr_t found;

    // 在特定 NODE 分配
    found = memblock_find_in_range_node(size, align, start, end, nid,
                        flags);
    if (found && !memblock_reserve(found, size))
        return found;

    // 尝试所有 NODE 去分配
    if (nid != NUMA_NO_NODE) {
        found = memblock_find_in_range_node(size, align, start,
                            end, NUMA_NO_NODE,
                            flags);
        if (found && !memblock_reserve(found, size))
            return found;
    }

    return 0;
}

phys_addr_t memblock_phys_alloc_range(phys_addr_t size,
                        phys_addr_t align,
                        phys_addr_t start,
                        phys_addr_t end)
{
    return memblock_alloc_range_nid(size, align, start, end, NUMA_NO_NODE);
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

int memblock_reserve(phys_addr_t base, phys_addr_t size)
{
    phys_addr_t end = base + size - 1;
    printk("memblock_reserve: [0x%16x-0x%16x] size 0x%16x\n", base, end, size);

    return memblock_add_range(&memblock.reserved, base, size, MAX_NUMNODES, MEMBLOCK_NONE);
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

