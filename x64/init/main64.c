#include "asm/system.h"
#include "linux/kernel.h"
#include "linux/tty.h"
#include "asm/e820.h"
#include "test/memblock.h"

void main_x64(void)
{
    console_init();

    e820__memory_setup();
    e820__memblock_setup();

    test_memblock();
    while (1);
}

