#include "linux/kernel.h"
#include "asm/system.h"
#include "idt.h"
#include "pic.h"

static idt_item_t idt_table[256] = {0};
static idtr_data_t idtr_data;

extern void clock_handler_entry(void);
void clock_interrupt_handler(void)
{
    printk("*");
    send_eoi(0x20);
}

// 通用中断/异常处理入口
void general_interrupt_handler(void)
{
    printk("%s: error!\n", __func__);

    while(1) {
        HLT();
    }

    return;
}

void install_idt_item(int index, int64 handler, short selector, char ist, char dpl)
{
    idt_item_t *item = &idt_table[index];
    item->offset0 = handler & 0xffff;
    item->selector = selector;
    item->ist = ist;
    item->type = 0b1110;  // 64-bit Interrupt Gate
    item->d_type = 0;
    item->dpl = dpl;
    item->present = 1;
    item->offset1 = (handler >> 16) & 0xffff;
    item->offset2 = (handler >> 32) & 0xffffffff;

    return;
}

// Vol.3A 6-20 Figure 6-8
void idt_init(void)
{
    int64 handler = (int64)general_interrupt_handler;

    for (int i = 0; i < 256; ++i) {
        // selector = (3 << 3)  // gdt[3] x64_code_descriptor
        // ist = 0;  // 不启用中断栈表
        install_idt_item(i, handler, 0x18, 0, 0);
    }

    install_idt_item(0x20, &clock_handler_entry, 0x18, 0, 0);

    idtr_data.limit = sizeof(idt_table);
    idtr_data.base = (int64)&idt_table;

    __asm__ __volatile__(
        "lidt idtr_data"
    );

    return;
}

