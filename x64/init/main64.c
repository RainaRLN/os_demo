#include "asm/system.h"
#include "linux/kernel.h"
#include "linux/tty.h"
#include "mm.h"

void main_x64(void)
{
    console_init();

    show_memory_map();

    BOCHS_DEBUG();
    while(1) {
        __asm__ __volatile__("hlt");
    }
}
