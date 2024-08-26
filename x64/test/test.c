#include "test/test.h"
#include "linux/kernel.h"
#include "mm.h"

void test_phymem_manage(void)
{
    int *p = get_free_page();
    printk("get 0x%8x\n", p);

    p = get_free_page();
    printk("get 0x%8x\n", p);

    printk("free 0x%8x\n", p);
    free_page(p);

    p = get_free_page();
    printk("get 0x%8x\n", p);
    return;
}

