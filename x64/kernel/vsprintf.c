#include "linux/kernel.h"

static char *number(char *str, long long num, char type, int fill_zero)
{
#define LEN 36
    char *p;
    char tmp_buf[LEN];
    unsigned long long n;
    int base;

    if (fill_zero) {
        for (int i = 0; i < LEN - 1; ++i) {
            tmp_buf[i] = '0';
        }
    }

    p = &tmp_buf[LEN-1];
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
        n = (unsigned long long)num;
        break;
    case 'x':
        base = 16;
        n = (unsigned long long)num;
        break;
    }

    if (n == 0) {
        *--p = '0';
    } else {
        do {
            *--p = "0123456789ABCDEF"[n % base];
        } while (n /= base);
    }

    if (fill_zero) {
        if (p > &tmp_buf[LEN-1-fill_zero]) {
            p = &tmp_buf[LEN-1-fill_zero];
        }
    }

    while (*p != '\0') {
        *str++ = *p++;
    }
#undef LEN
    return str;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
    char *str = buf;
    char *s;
    int fill_zero = 0;

    while (*fmt) {
        if (*fmt != '%') {
            *str++ = *fmt++;
            continue;
        }
        fmt++;

        while ((*fmt >= '0') && (*fmt <= '9')) {
            fill_zero = fill_zero * 10 + *(fmt++) - '0';
        }

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
            fill_zero = 0;
        case 'x':
            str = number(str, va_arg(args, unsigned long long), *fmt, fill_zero);
            break;
        default:
            *str++ = *fmt;
            break;
        }
        fmt++;
        fill_zero = 0;
    }
    *str = '\0';
    return str - buf;
}

