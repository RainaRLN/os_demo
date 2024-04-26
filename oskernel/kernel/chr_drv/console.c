#include "asm/io.h"
#include "linux/tty.h"
#include "string.h"

#define VID_MEM_BASE 0xB8000  // 显存起始位置
#define VID_MEM_SIZE 0x4000  // 显存大小
#define VID_MEM_END (VID_MEM_BASE + VID_MEM_SIZE)  // 显存结束位置

#define CRT_ADDR_REG 0x3D4
#define CRT_DATA_REG 0x3D5
#define CRT_START_ADDR_H 0x0C  // 显示起始 pixel 位置 H
#define CRT_START_ADDR_L 0x0D  // 显示起始 pixel 位置 L
#define CRT_CURSOR_H 0x0E  // 光标位置 H
#define CRT_CURSOR_L 0x0F  // 光标位置 L

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define ROW_SIZE (SCREEN_WIDTH * 2)  // 每行字节数
#define SCREEN_SIZE (ROW_SIZE * SCREEN_HEIGHT)  // 一屏字节数

static uint screen_base;  // 当前显示器开始的内存位置
static uint cur_pos;  // 当前光标在内存的位置
static uint cur_x, cur_y;  // 当前光标位置

// 设置当前显示器开始位置
static void set_screen(void)
{
    out_byte(CRT_ADDR_REG, CRT_START_ADDR_H);
    out_byte(CRT_DATA_REG, (((screen_base - VID_MEM_BASE) / 2)  >> 8) & 0xff);
    out_byte(CRT_ADDR_REG, CRT_START_ADDR_L);
    out_byte(CRT_DATA_REG, ((screen_base - VID_MEM_BASE) / 2) & 0xff);
    return;
}

static void set_cursor(void)
{
    out_byte(CRT_ADDR_REG, CRT_CURSOR_H);
    out_byte(CRT_DATA_REG, (((cur_pos - VID_MEM_BASE) / 2) >> 8) & 0xff);
    out_byte(CRT_ADDR_REG, CRT_CURSOR_L);
    out_byte(CRT_DATA_REG, ((cur_pos - VID_MEM_BASE) / 2) & 0xff);
    return;
}

void console_clear()
{
    u16 *ptr = (u16 *)VID_MEM_BASE;
    while (ptr < (u16 *)VID_MEM_END) {
        *ptr++ = 0x0720;  // 黑底白字空格
    }

    screen_base = VID_MEM_BASE;
    cur_pos = VID_MEM_BASE;
    cur_x = 0;
    cur_y = 0;
    set_cursor();
    set_screen();
    return;
}

static void scroll_up()
{
    if (screen_base + SCREEN_SIZE + ROW_SIZE < VID_MEM_END) {
        u16 *ptr = (u16 *)(screen_base + SCREEN_SIZE);
        for (size_t i = 0; i < SCREEN_WIDTH; i++) {
            *ptr++ = 0x0720;
        }
        screen_base += ROW_SIZE;
        cur_pos += ROW_SIZE;
    } else {
        memcpy((char *)VID_MEM_BASE, (char *)(screen_base + ROW_SIZE), SCREEN_SIZE - ROW_SIZE);
        cur_pos = VID_MEM_BASE + (cur_y * 80 + cur_x) * 2;
        screen_base = VID_MEM_BASE;
        u16 *ptr = (u16 *)(screen_base + SCREEN_SIZE - ROW_SIZE);
        while (ptr < (u16 *)VID_MEM_END) {
            *ptr++ = 0x0720;
        }
    }
    set_screen();
    return;
}

static void console_lf()
{
    if (cur_y + 1 < SCREEN_HEIGHT) {
        cur_y++;
        cur_pos += ROW_SIZE;
    } else {
        scroll_up();
    }
    return;
}

static void console_cr()
{
    cur_pos -= cur_x * 2;
    cur_x = 0;
    return;
}

static void console_bs()
{
    if (cur_x == 0)
        return;

    cur_x--;
    cur_pos -= 2;
    *(u16 *)cur_pos = 0x0720;
    return;
}

static void console_del()
{
    *(u16 *)cur_pos = 0x0720;
    return;
}

void console_write(char *buf, u32 count)
{
    char c;
    while (count--) {
        c = *buf++;
        switch (c) {
        case '\b':
            console_bs();
            break;
        case '\n':
            console_lf();
            console_cr();
            break;
        case '\r':
            console_cr();
            break;
        case 0x7F:  // DEL
            console_del();
            break;
        default:
            *(char *)cur_pos++ = c;
            *(char *)cur_pos++ = 0x07;
            cur_x++;
            if (cur_x >= SCREEN_WIDTH) {
                cur_x -= SCREEN_WIDTH;
                cur_pos -= ROW_SIZE;
                console_lf();
            }
            break;
        }
    }
    set_cursor();
    return;
}

void console_init(void)
{
    console_clear();
}

