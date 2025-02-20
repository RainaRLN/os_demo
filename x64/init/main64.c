#include "asm/system.h"
#include "linux/kernel.h"
#include "linux/tty.h"
#include "test/test.h"
#include "idt.h"
#include "pic.h"
#include "asm/e820.h"

void main_x64(void)
{
    console_init();

    e820__memory_setup();
    e820__memblock_setup();

    pic_init();
    idt_init();

    // test_div_err();
    // test_pf_err();
    test_int1f();

    STI();
    while(1) {
        HLT();
    }
}

