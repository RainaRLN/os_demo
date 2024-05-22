#include "linux/kernel.h"
#include "linux/tty.h"
#include "enter_x64.h"

void kernel_main(void)
{
    console_init();

    if (!x64_cpu_check()) {
        printk("Unsupported CPU.\nEnd\n");
    }

    enter_x64();

    while (1);
}
