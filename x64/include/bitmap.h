#ifndef _BITMAP_H_
#define _BITMAP_H_

#include "linux/types.h"

typedef struct {
    u8 *bits;  // 位图数据
    u32 length;  // 位图长度
} bitmap_t;


void bitmap_init(bitmap_t *map, uchar *buf, u32 len);
void bitmap_set(bitmap_t *map, u32 index, bool value);
int bitmap_scan(bitmap_t *map, u32 count);

#endif  // _BITMAP_H_

