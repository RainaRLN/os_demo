/* 参考 linux-0.12/lib/malloc.c */

#include "linux/kernel.h"
#include "asm/system.h"
#include "linux/types.h"
#include "mm.h"

// 存储桶描述符结构
struct bucket_desc {
    void *page;  // 该桶描述符对应的物理页
    struct bucket_desc *next;  // 下一个描述符指针
    void *freeptr;  // 指向本桶空闲内存块
    unsigned short refcnt;  // 引用计数
    unsigned short bucket_size;  // 本描述符的内存块大小
};

// 存储桶目录
struct _bucket_dir {
    int size;  // bucket 大小
    struct bucket_desc *chain;  // 该存储桶目录项的桶描述符链表
};

// 存储桶目录列表
struct _bucket_dir bucket_dir[] = {
    {16, (struct bucket_desc *)0},  // 16 字节大小的内存块
    {32, (struct bucket_desc *)0},
    {64, (struct bucket_desc *)0},
    {128, (struct bucket_desc *)0},
    {256, (struct bucket_desc *)0},
    {512, (struct bucket_desc *)0},
    {1024, (struct bucket_desc *)0},
    {2048, (struct bucket_desc *)0},
    {4096, (struct bucket_desc *)0},  // 最大支持 4096 (1 页)大小的内存块
    {0, (struct bucket_desc *)0},  // End of list marker
};

// 空闲存储桶描述符链表
struct bucket_desc *free_bucket_desc = (struct bucket_desc *)0;

static inline void init_bucket_desc(void)
{
    struct bucket_desc *bdesc, *first;
    int i;

    // 申请一个物理页，存储桶描述符
    first = bdesc = (struct bucket_desc *)get_free_page();
    if (!bdesc) {
        printk("Out of memory in %s\n", __func__);
        HLT();
    }
    for (i = PAGE_SIZE / sizeof(struct bucket_desc); i > 1; i--) {
        bdesc->next = bdesc + 1;
        bdesc++;
    }

    // 将这些桶描述符插入 free_bucket_desc 头部
    bdesc->next = free_bucket_desc;
    free_bucket_desc = first;  // free_bucket_desc 指向空桶描述符链表的头
    return;
}

void *kmalloc(size_t len)
{
    struct _bucket_dir *bdir;
    struct bucket_desc *bdesc;
    void *retval;

    // 搜索存储桶目录，找到桶字节数大于请求字节数的桶描述符链表
    for (bdir = bucket_dir; bdir->size; bdir++) {
        if (bdir->size >= len) {
            break;
        }
    }
    // End of list marker，说明超出了最大内存大小限制
    if (!bdir->size) {
        printk("%s called with impossibly large argument (%d)\n", 
                __func__, len);
        HLT();
    }

    // CLI();  // Avoid race conditions

    // 先在 bdir->chain 搜索有空内存块的桶描述符
    for (bdesc = bdir->chain; bdesc; bdesc = bdesc->next) {
        if (bdesc->freeptr) {
            break;
        }
    }
    // 没有找到有空闲内存块的桶描述符，取一个空闲桶描述符为其分配一个 page
    if (!bdesc) {
        char *cp;
        int i;

        // 当前无可用的空闲桶描述符
        // 申请一个页面，新建一批空闲桶描述符链表
        if (!free_bucket_desc) {
            init_bucket_desc();
        }
        // 取出第一个空闲桶描述符
        bdesc = free_bucket_desc;
        // free_bucket_desc 指向下一个空闲桶描述符
        free_bucket_desc = bdesc->next;

        // 初始化桶描述符
        bdesc->refcnt = 0;
        bdesc->bucket_size = bdir->size;
        // 为该桶描述符申请一个 page
        bdesc->page = (void *)(cp = (char *)get_free_page());
        if (!cp) {
            printk("Out of memory in %s\n", __func__);
            HLT();
        }

        bdesc->freeptr = (void *)cp;  // 空闲指针指向该页首地址
        // 分割当前空闲页, 每个空闲块头部记录下一个空闲块地址
        for (i = PAGE_SIZE / bdir->size; i > 1; i--) {
            char *next_freeptr = cp + bdir->size;  // 计算下一个空闲内存块地址
            *((char **)cp) = next_freeptr;  // 记录下一个内存块地址
            cp += bdir->size;  // 指向下一个内存块
        }
        *((char **)cp) = 0;  // 最后一个内存块，没有下一个空闲内存块了

        // 将当前桶描述符插入 bdir->chain 头部
        bdesc->next = bdir->chain;
        bdir->chain = bdesc;
    }

    retval = (void *)bdesc->freeptr;  // 返回 freeptr 指向的内存块
    bdesc->freeptr = *((void **)retval);  // freeptr 指向下一个空闲内存块
    bdesc->refcnt++;  // 当前 page 引用计数 +1

    // STI();  // OK, we're safe again

    return (retval);  // 返回申请到的内存块
}

// 可指定 size 加速查找对应桶描述符的速度
// 只会查找能容下 size 大小的桶描述符链表
void kfree_s(void *obj, int size)
{
    void *page;
    struct _bucket_dir *bdir;
    struct bucket_desc *bdesc, *prev;

    // 计算该内存块所在 page
    page = (void *)((unsigned long)obj & 0xfffff000);

    // 查找管理该页的桶描述符
    for (bdir = bucket_dir; bdir->size; bdir++) {
        prev = 0;
        // 当前桶描述符链表的大小不足以容纳该内存块，下一个
        if (bdir->size < size) {
            continue;
        }
        for (bdesc = bdir->chain; bdesc; bdesc = bdesc->next) {
            if (bdesc->page == page) {
                goto found;  // 找到对应的桶描述符了
            }
            prev = bdesc;  // 记录上一个桶描述符
        }
    }
    // 遍历完都没有找到
    printk("Bad address passed to %s\n", __func__);
    return;

found:
    // CLI();  // To avoid race conditions

    // 将要释放的内存块插入 bdesc->freeptr 头部
    *((void **)obj) = bdesc->freeptr;
    bdesc->freeptr = obj;

    bdesc->refcnt--;  // 引用计数 -1

    // 这个桶描述符可以回收了
    if (bdesc->refcnt == 0) {
        // 这了是为了确保 prev 仍然正确，没有被其他程序中断篡改
        if ((prev && (prev->next != bdesc)) ||  // 当前 desc 不是第一个
            (!prev && (bdir->chain != bdesc))) {  // 当前 desc 是首个
            // 如果被篡改了，重新查找 prev
            for (prev = bdir->chain; prev; prev = prev->next) {
                if (prev->next == bdesc) {
                    break;  // 找到 prev 了
                }
            }
        }
        // 把当前 desc 从 chain 中删除
        if (prev) {
            prev->next = bdesc->next;
        } else {
            if (bdir->chain != bdesc) {  // 没找到 prev，something wrong
                printk("malloc bucket chains corrupted\n");
                return;
            }
            bdir->chain = bdesc->next;
        }
        // 释放对应的物理页
        free_page(bdesc->page);
        // 把描述符添加到 free_bucket_desc 头部
        bdesc->next = free_bucket_desc;
        free_bucket_desc = bdesc;
    }

    // STI();
    return;
}

