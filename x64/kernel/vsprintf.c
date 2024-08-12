#include "linux/kernel.h"

static char *number(char *str, int num, char type, int fill_zero)
{

    char *p;
    char tmp_buf[36];
    unsigned int n;
    int base;

    if (fill_zero) {
        for (int i = 27; i < 35; ++i) {
            tmp_buf[i] = '0';
        }
    }

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

    if (fill_zero) {
        if (p > &tmp_buf[35-fill_zero]) {
            p = &tmp_buf[35-fill_zero];
        }
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
    int fill_zero = 0;

    while (*fmt) {
        if (*fmt != '%') {
            *str++ = *fmt++;
            continue;
        }
        fmt++;

        switch (*fmt) {
        case '2'...'8':
            fill_zero = *fmt - '0';
            fmt++;
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
        case 'x':
            str = number(str, va_arg(args, unsigned int), *fmt, fill_zero);
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

