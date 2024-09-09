#include "asm/system.h"
#include "linux/kernel.h"
#include "linux/tty.h"
#include "mm.h"
#include "test/test.h"
#include "idt.h"

void main_x64(void)
{
    console_init();

    show_memory_map();

    phy_memory_init();

    idt_init();

    test_div_err();

    while(1) {
        HLT();
    }
}

