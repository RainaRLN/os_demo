#include "mm.h"
#include "linux/kernel.h"
#include "string.h"

#define E820_ARDS_NUM_ADDR 0x7e00
#define E820_ARDS_ADDR 0x7e02

char *ards_type[] = {
    "AddressRangeMemory",  // 1
    "AddressRangeReserved",  // 2
    "Undefined",  // others
};

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

