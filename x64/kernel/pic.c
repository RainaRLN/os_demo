#include "asm/io.h"
#include "pic.h"

void send_eoi(int idt_index)
{
    if (idt_index >= 0x20 && idt_index < 0x28) {
        out_byte(0x20, 0x20);
    } else if (idt_index >= 0x28 && idt_index < 0x30) {
        out_byte(0x20, 0x20);
        out_byte(0xA0, 0x20);
    }

    return;
}

void init_8259a(void)
{
    // 设置主从的 ICW1
    out_byte(0x20, 0x11);  // 边沿触发、级联、使用 ICW4
    out_byte(0xA0, 0x11);

    // 设置主从的 ICW2
    out_byte(0x21, 0x20);  // 起始中断向量号 (0~31 已经被占用或保留)
    out_byte(0xA1, 0x28);

    // 设置主从的 ICW3
    out_byte(0x21, 0x04);  // 通过 IRQ2 与从片相连
    out_byte(0xA1, 0x02);  // 与主片 IRQ2 相连

    // 设置主从的 ICW4
    out_byte(0x21, 0x03);  // 8086 模式，自动 EOI
    out_byte(0xA1, 0x03);

    // 设置主从的 OCW1
    out_byte(0x21, 0xFD);  // 只接受主片 IRQ1 键盘中断
    out_byte(0xA1, 0xFF);  // 屏蔽从片所有中断

    return;
}

void pic_init(void)
{
    init_8259a();

    return;
}
