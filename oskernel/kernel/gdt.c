#include "linux/gdt.h"
#include "linux/kernel.h"
#include "string.h"

// Vol.3A 3-16
// In IA-32e mode, a segment descriptor table can contain up to
// 8192(2^13) 8-byte descriptors.
#define GDT_SIZE 256  // 256 个够用了
gdt_item_t gdt[GDT_SIZE] = {0};

gdtr_data_t gdtr_data;

// Vol.3A 5-3
static void install_x64_code_descriptor(int gdt_index)
{
    gdt_item_t *item = &gdt[gdt_index];

    // base address and limit field is ignored
    item->limit_low = 0;
    item->base_low = 0;
    item->seg_type = 0b1000;  // Execute-Only
    item->d_type = 1;
    item->dpl = 0;
    item->present = 1;
    item->limit_high = 0;
    item->available = 0;
    item->long_mode = 1;  // x64-bit mode
    item->db = 0;  // must be 0 x64-bit mode
    item->granularity = 0;
    item->base_high = 0;

    return;
}

static void install_x64_data_descriptor(int gdt_index)
{
    gdt_item_t *item = &gdt[gdt_index];

    item->limit_low = 0;
    item->base_low = 0;
    item->seg_type = 0b0010;  // Read/Write
    item->d_type = 1;
    item->dpl = 0;
    item->present = 1;
    item->limit_high = 0;
    item->available = 0;
    item->long_mode = 1;
    item->db = 0;
    item->granularity = 0;
    item->base_high = 0;

    return;
}

void install_x64_descriptor(void)
{
    __asm__ __volatile__(
        "sgdt gdtr_data"
    );
    printk("gdt base: 0x%x, gdt limit: 0x%x\n", gdtr_data.base, gdtr_data.limit);

    memcpy(&gdt, (void *)(gdtr_data.base), gdtr_data.limit);
    install_x64_code_descriptor(3);
    install_x64_data_descriptor(4);

    gdtr_data.base = (int)(&gdt);
    gdtr_data.limit = sizeof(gdt);
    printk("gdt base: 0x%x, gdt limit: 0x%x\n", gdtr_data.base, gdtr_data.limit);

    __asm__ __volatile__(
        "lgdt gdtr_data"
    );
}
