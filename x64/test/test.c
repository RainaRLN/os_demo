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

void test_vm_manage(void)
{
    void *p1 = kmalloc(1024);
    printk("p1 0x%8x\n", p1);

    void *p2 = kmalloc(2 * 1024);
    printk("p2 0x%8x\n", p2);
    void *p22 = kmalloc(2 * 1024);
    printk("p22 0x%8x\n", p22);

    printk("free p1 0x%8x\n", p1);
    kfree_s(p1, 1024);

    void *p3 = kmalloc(3 * 1024);
    printk("p3 0x%8x\n", p3);
    void *p33 = kmalloc(3 * 1024);
    printk("p33 0x%8x\n", p33);

    p1 = kmalloc(1 * 1024);
    printk("p1 0x%8x\n", p1);
}

void test_pf_err(void)
{
    int *p = (int *)0xdeadbeef;
    *p = 1;
    return;
}

void test_div_err(void)
{
    int i = 10 / 0;
    return;
}

