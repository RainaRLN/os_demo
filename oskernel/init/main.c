#include "linux/kernel.h"
#include "linux/tty.h"

void kernel_main(void)
{
    console_init();
    printk("%s:%d Hello world\n", __func__, __LINE__);
}
