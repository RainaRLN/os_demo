#include "linux/kernel.h"
#include "asm/e820.h"
#include "string.h"
#include "linux/memblock.h"

#define E820_ARDS_NUM_ADDR (0x7e00)

static struct e820_table e820_table_init;
struct e820_table *e820_table = &e820_table_init;

static void e820_print_type(enum e820_type type)
{
    printk("type %u ", type);
	switch (type) {
	case E820_TYPE_RAM:		/* Fall through: */
	case E820_TYPE_RESERVED_KERN:	printk("usable\n");			break;
	case E820_TYPE_RESERVED:	printk("reserved\n");			break;
	case E820_TYPE_ACPI:		printk("ACPI data\n");			break;
	case E820_TYPE_NVS:		printk("ACPI NVS\n");			break;
	case E820_TYPE_UNUSABLE:	printk("unusable\n");			break;
	case E820_TYPE_PMEM:		/* Fall through: */
	case E820_TYPE_PRAM:		printk("persistent\n");	break;
	default:			printk("unknown\n");		break;
	}

    return;
}

void e820__print_table(void)
{
    int i = 0;
    u64 total_usable = 0;
    for (i = 0; i < e820_table->nr_entries; i++) {
        printk("mem: base 0x%16x, length 0x%16x, ",
                    e820_table->entries[i].addr, e820_table->entries[i].size);
        e820_print_type(e820_table->entries[i].type);
        if (E820_TYPE_RAM == e820_table->entries[i].type) {
            total_usable += e820_table->entries[i].size;
        }
    }

    printk("total usable: %u MB %u KB %u B\n", 
            total_usable >> 20,
            (total_usable & 0xFFFFF) >> 10,
            total_usable & 0x3FF);

    return;
}

void e820__memory_setup(void)
{
    memcpy(e820_table, (void *)E820_ARDS_NUM_ADDR, sizeof(struct e820_table));
    e820__print_table();

    return;
}

void e820__memblock_setup(void)
{
    for (int i = 0; i < e820_table->nr_entries; ++i) {
        struct e820_entry *entry = &e820_table->entries[i];

        if (entry->type != E820_TYPE_RAM && entry->type != E820_TYPE_RESERVED_KERN) {
            // 非 usable 内存段，跳过
            continue;
        }

        memblock_add(entry->addr, entry->size);
    }

    memblock_dump_all();

    return;
}

