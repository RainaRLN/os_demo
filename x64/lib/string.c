#include "string.h"

char *strcpy(char *dest, const char *src)
{
    char *p = dest;
    while (*src != '\0') {
        *p++ = *src++;
    }
    *p = '\0';
    return dest;
}

size_t strlen(const char *s)
{
    char *p = (char *)s;
    while (*p != '\0') {
        p++;
    }
    return (p - s);
}

int strcmp(const char *s1, const char *s2)
{
    while ((*s1 == *s2) && (*s1 != '\0') && (*s2 != '\0')) {
        s1++;
        s2++;
    }
    return (*s1 - *s2);
}

void *memset(void *s, int c, size_t n)
{
    char *p = s;
    while (n--) {
        *p = c;
    }
    return s;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    char *p = dest;
    while (n--) {
        *p++ = *((char *)(src++));
    }
    return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
    unsigned char *d = dest;
    const unsigned char *s = src;

    // d 在 s 前面 || d 在 s+n 后面，没有重叠
    // 直接从前往后复制
    if (d <= s || d >= s + n) {
        while (n-- > 0) {
            *d++ = *s++;
        }
    } else {  // 从后往前复制
        while (n-- > 0)
            d[n] = s[n];
    }

    return dest;
}

