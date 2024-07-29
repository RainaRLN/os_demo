#include "asm/system.h"
#include "linux/kernel.h"
#include "linux/tty.h"

void main_x64(void)
{
    console_init();

    BOCHS_DEBUG();
    while(1);
}
