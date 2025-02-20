#ifndef _LINUX_KERNEL_H_
#define _LINUX_KERNEL_H_

#include "stdarg.h"
#include "asm/system.h"

#define __WARN_printk(x) do { \
    printk("%s:%d %s WARN(" #x ")\n", __FILE__, __LINE__, __func__); \
} while (0)

#define WARN_ON(condition) ({ \
    int __ret_warn_on = !!(condition); \
    if (__ret_warn_on) \
        __WARN_printk(condition); \
    (__ret_warn_on); \
})

#define BUG(x) do { \
    printk("%s:%d %s BUG(" #x ")\n", __FILE__, __LINE__, __func__); \
    HLT(); \
} while (0)

#define BUG_ON(condition) do { if (condition) BUG(condition); } while(0)

#define WARN_ONCE(condition, format...) ({ \
    int __ret_warn_once = !!(condition); \
    if (__ret_warn_once) \
        printk(format); \
    (__ret_warn_once); \
})

int vsprintf(char *buf, const char *fmt, va_list args);
int printk(const char *fmt, ...);

#endif  // _LINUX_KERNEL_H_

