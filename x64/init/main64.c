#include "asm/system.h"
#include "linux/kernel.h"
#include "linux/tty.h"
#include "mm.h"
#include "test/test.h"
#include "idt.h"
#include "pic.h"

void main_x64(void)
{
    console_init();

    show_memory_map();

    phy_memory_init();

    pic_init();
    idt_init();

    STI();
    while(1) {
        HLT();
    }
}

