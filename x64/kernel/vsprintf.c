#include "linux/kernel.h"

static char *number(char *str, int num, char type)
{

    char *p;
    char tmp_buf[36];
    unsigned int n;
    int base;
    p = &tmp_buf[35];
    *p = '\0';
    switch (type) {
    case 'd':
        base = 10;
        if (num < 0) {
            *str++ = '-';
            n = -num;
        } else {
            n = num;
        }
        break;
    case 'u':
        base = 10;
        n = (unsigned int)num;
        break;
    case 'x':
        base = 16;
        n = (unsigned int)num;
        break;
    }

    if (n == 0) {
        *--p = '0';
    } else {
        do {
            *--p = "0123456789ABCDEF"[n % base];
        } while (n /= base);
    }

    while (*p != '\0') {
        *str++ = *p++;
    }
    return str;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
    char *str = buf;
    char *s;

    while (*fmt) {
        if (*fmt != '%') {
            *str++ = *fmt++;
            continue;
        }
        fmt++;

        switch (*fmt) {
        case 'c':
            *str++ = (unsigned char)va_arg(args, unsigned char);
            break;
        case 's':
            s = va_arg(args, char *);
            while (*s) {
                *str++ = *s++;
            }
            break;
        case 'd':
        case 'u':
        case 'x':
            str = number(str, va_arg(args, unsigned int), *fmt);
            break;
        default:
            *str++ = *fmt;
            break;
        }
        fmt++;
    }
    *str = '\0';
    return str - buf;
}

