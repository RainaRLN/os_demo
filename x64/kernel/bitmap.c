#include "bitmap.h"
#include "string.h"

void bitmap_make(bitmap_t *map, uchar *buf, u32 len)
{
    map->bits = buf;
    map->length = len;
    return;
}

void bitmap_init(bitmap_t *map, uchar *buf, u32 len)
{
    memset(buf, 0, len);
    bitmap_make(map, buf, len);
    return;
}

bool bitmap_test(bitmap_t *map, u32 index)
{
    u32 byte = index / 8;
    u8 bit = index % 8;

    return (map->bits[byte] & (1 << bit));
}

void bitmap_set(bitmap_t *map, u32 index, bool value)
{
    u32 byte = index / 8;  // 所在字节
    u8 bit = index % 8;  // 字节中的哪一位

    if (value) {
        map->bits[byte] |= (1 << bit);
    } else {
        map->bits[byte] &= ~(1 << bit);
    }
    return;
}

// 从位图中找到连续的 count 位
int bitmap_scan(bitmap_t *map, u32 count)
{
    int start = -1;
    u32 bits_num = map->length * 8;   // 总 bit 数
    u32 cur_bit = 0;
    u32 num = 0;

    while (bits_num-- > 0) {
        if (!bitmap_test(map, cur_bit)) {
            num++;
        } else {
            num = 0;
        }

        cur_bit++;

        if (num == count) {
            start = cur_bit - count;
            break;
        }
    }

    if (start == -1) {
        return start;
    }

    // 将找到的位置 1
    cur_bit = start;
    while (num--) {
        bitmap_set(map, cur_bit, 1);
        cur_bit++;
    }

    return start;
}

