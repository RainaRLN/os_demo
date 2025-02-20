#ifndef _ASM_E820_H_
#define _ASM_E820_H_

#include "linux/types.h"
#define E820_MAX_ENTRIES 128

/*
 * These are the E820 types known to the kernel:
 */
enum e820_type {
	E820_TYPE_RAM		= 1,
	E820_TYPE_RESERVED	= 2,
	E820_TYPE_ACPI		= 3,
	E820_TYPE_NVS		= 4,
	E820_TYPE_UNUSABLE	= 5,
	E820_TYPE_PMEM		= 7,

	/*
	 * This is a non-standardized way to represent ADR or
	 * NVDIMM regions that persist over a reboot.
	 *
	 * The kernel will ignore their special capabilities
	 * unless the CONFIG_X86_PMEM_LEGACY=y option is set.
	 *
	 * ( Note that older platforms also used 6 for the same
	 *   type of memory, but newer versions switched to 12 as
	 *   6 was assigned differently. Some time they will learn... )
	 */
	E820_TYPE_PRAM		= 12,

	/*
	 * Reserved RAM used by the kernel itself if
	 * CONFIG_INTEL_TXT=y is enabled, memory of this type
	 * will be included in the S3 integrity calculation
	 * and so should not include any memory that the BIOS
	 * might alter over the S3 transition:
	 */
	E820_TYPE_RESERVED_KERN	= 128,
};


struct e820_entry {
    u64 addr;
    u64 size;
    enum e820_type type;
} __attribute__((packed));

struct e820_table {
    u16 nr_entries;
    struct e820_entry entries[E820_MAX_ENTRIES];
};

void e820__memory_setup(void);
void e820__memblock_setup(void);
#endif  // _ASM_E820_H_

