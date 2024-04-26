#include "linux/kernel.h"
#include "linux/tty.h"

#define PRINTK_BUF_LEN 1024
static char printk_buf[PRINTK_BUF_LEN];

int printk(const char *fmt, ...)
{
    va_list args;
    int count;

    va_start(args, fmt);
    count = vsprintf(printk_buf, fmt, args);
    va_end(args);

    console_write(printk_buf, count);

    return count;
}

