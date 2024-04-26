#ifndef _LINUX_KERNEL_H_
#define _LINUX_KERNEL_H_

#include "stdarg.h"

int vsprintf(char *buf, const char *fmt, va_list args);
int printk(const char *fmt, ...);

#endif  // _LINUX_KERNEL_H_

