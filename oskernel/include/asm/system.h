#ifndef _ASM_SYSTEM_H_
#define _ASM_SYSTEM_H_
#include "linux/kernel.h"

#ifdef DEBUG
#define BOCHS_DEBUG() do { \
    printk("%s:%d\n", __FILE__, __LINE__); \
    __asm__ __volatile__("xchg bx, bx"); \
} while (0)
#else
#define BOCHS_DEBUG()
#endif  // DEBUG


#endif  // _ASM_SYSTEM_H_

