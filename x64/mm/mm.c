#include "mm.h"
#include "linux/kernel.h"
#include "asm/system.h"
#include "string.h"
#include "bitmap.h"

#define E820_ARDS_NUM_ADDR 0x7e00
#define E820_ARDS_ADDR 0x7e02
#define BITMAP_ADDR 0x500

#define PHY_MEM_USE_START  0xa00000

enum zone_type {
    ZONE_MEMORY = 1,
    ZONE_RESERVED,
    ZONE_UNDEFINED,
};

char *ards_type[] = {
    "AddressRangeMemory",  // 1
    "AddressRangeReserved",  // 2
    "Undefined",  // others
};

physics_memory_map_t physics_memory_map = {0};

void phy_memory_init(void)
{
    ards_item_t *p_ards = (ards_item_t *)E820_ARDS_ADDR;
    unsigned short e820_ards_num = *((unsigned short *)E820_ARDS_NUM_ADDR);

    for (int i = 0; i < e820_ards_num; ++i) {
        ards_item_t *p_cur_ards = p_ards + i;

        // 只管低 4G，PHY_MEM_USE_START 所在的那片可用区域
        unsigned int zone_start = p_cur_ards->base_addr_low;
        unsigned int zone_end = zone_start + p_cur_ards->length_low;
        if (zone_start < PHY_MEM_USE_START && zone_end > PHY_MEM_USE_START && 
                p_cur_ards->type == ZONE_MEMORY) {
            physics_memory_map.addr_base = PHY_MEM_USE_START;
            physics_memory_map.pages_total = (zone_end - PHY_MEM_USE_START) >> 12;  // 多少个 4K 页面
            break;
        }
    }

    if (physics_memory_map.pages_total == 0) {
        printk("[%s:%d] %s no valid physics memory\n", __FILE__, __LINE__, __func__);
        return;
    }

    u32 bitmap_len = physics_memory_map.pages_total / 8;  // 一个 bit 管理一个 4K 页

    physics_memory_map.bitmap_buf = (uchar *)(BITMAP_ADDR);
    bitmap_init(&physics_memory_map.bitmap, physics_memory_map.bitmap_buf, bitmap_len);

    return;
}

void *get_free_page(void)
{
    int ret;
    int index = bitmap_scan(&physics_memory_map.bitmap, 1);
    if (index == -1) {
        printk("%s error!\n", __func__);
        HLT();
    }

    ret = physics_memory_map.addr_base + (index << 12);
    return (void *)ret;
}

void free_page(void *p)
{
    int index = ((int)p - physics_memory_map.addr_base) >> 12;
    if (index >= physics_memory_map.bitmap.length) {
        printk("%s error\n", __func__);
        HLT();
    }

    bitmap_set(&physics_memory_map.bitmap, index, 0);
    return;
}

void show_memory_map(void)
{
    ards_item_t *p_ards = (ards_item_t *)E820_ARDS_ADDR;
    unsigned short e820_ards_num = *((unsigned short *)E820_ARDS_NUM_ADDR);

    printk("====== memory check info =====\n");
    for (int i = 0; i < e820_ards_num; ++i) {
        ards_item_t *p_cur_ards = p_ards + i;
        // TODO: 当前 printk 不支持 6 个以上参数，所以分开
        printk("%u, ", i);
        printk("0x%8x_%8x, 0x%8x_%8x, ",
                p_cur_ards->base_addr_high, p_cur_ards->base_addr_low, 
                p_cur_ards->length_high, p_cur_ards->length_low);
        printk("%u %s\n", p_cur_ards->type, 
                ards_type[(p_cur_ards->type > 3 ? 3 : p_cur_ards->type) - 1]);
    }
    printk("====== memory check info =====\n");
}

