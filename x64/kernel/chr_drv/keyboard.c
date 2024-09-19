#include "linux/kernel.h"
#include "linux/types.h"
#include "asm/io.h"
#include "pic.h"

#define INV 0  // 字符不可见

typedef enum {
    /* 0x00 */ KEY_NONE,
    /* 0x01 */ KEY_ESC,
    /* 0x02 */ KEY_1,
    /* 0x03 */ KEY_2,
    /* 0x04 */ KEY_3,
    /* 0x05 */ KEY_4,
    /* 0x06 */ KEY_5,
    /* 0x07 */ KEY_6,
    /* 0x08 */ KEY_7,
    /* 0x09 */ KEY_8,
    /* 0x0A */ KEY_9,
    /* 0x0B */ KEY_0,
    /* 0x0C */ KEY_MINUS,
    /* 0x0D */ KEY_EQUAL,
    /* 0x0E */ KEY_BACKSPACE,
    /* 0x0F */ KEY_TAB,
    /* 0x10 */ KEY_Q,
    /* 0x11 */ KEY_W,
    /* 0x12 */ KEY_E,
    /* 0x13 */ KEY_R,
    /* 0x14 */ KEY_T,
    /* 0x15 */ KEY_Y,
    /* 0x16 */ KEY_U,
    /* 0x17 */ KEY_I,
    /* 0x18 */ KEY_O,
    /* 0x19 */ KEY_P,
    /* 0x1A */ KEY_BRACKET_L,
    /* 0x1B */ KEY_BRACKET_R,
    /* 0x1C */ KEY_ENTER,
    /* 0x1D */ KEY_CTRL_L,
    /* 0x1E */ KEY_A,
    /* 0x1F */ KEY_S,
    /* 0x20 */ KEY_D,
    /* 0x21 */ KEY_F,
    /* 0x22 */ KEY_G,
    /* 0x23 */ KEY_H,
    /* 0x24 */ KEY_J,
    /* 0x25 */ KEY_K,
    /* 0x26 */ KEY_L,
    /* 0x27 */ KEY_SEMICOLON,
    /* 0x28 */ KEY_QUOTE,
    /* 0x29 */ KEY_BACKQUOTE,
    /* 0x2A */ KEY_SHIFT_L,
    /* 0x2B */ KEY_BACKSLASH,
    /* 0x2C */ KEY_Z,
    /* 0x2D */ KEY_X,
    /* 0x2E */ KEY_C,
    /* 0x2F */ KEY_V,
    /* 0x30 */ KEY_B,
    /* 0x31 */ KEY_N,
    /* 0x32 */ KEY_M,
    /* 0x33 */ KEY_COMMA,
    /* 0x34 */ KEY_POINT,
    /* 0x35 */ KEY_SLASH,
    /* 0x36 */ KEY_SHIFT_R,
    /* 0x37 */ KEY_STAR,
    /* 0x38 */ KEY_ALT_L,
    /* 0x39 */ KEY_SPACE,
    /* 0x3A */ KEY_CAPSLOCK,
    /* 0x3B */ KEY_F1,
    /* 0x3C */ KEY_F2,
    /* 0x3D */ KEY_F3,
    /* 0x3E */ KEY_F4,
    /* 0x3F */ KEY_F5,
    /* 0x40 */ KEY_F6,
    /* 0x41 */ KEY_F7,
    /* 0x42 */ KEY_F8,
    /* 0x43 */ KEY_F9,
    /* 0x44 */ KEY_F10,
    /* 0x45 */ KEY_NUMLOCK,
    /* 0x46 */ KEY_SCRLOCK,
    /* 0x47 */ KEY_PAD_7,
    /* 0x48 */ KEY_PAD_8,
    /* 0x49 */ KEY_PAD_9,
    /* 0x4A */ KEY_PAD_MINUS,
    /* 0x4B */ KEY_PAD_4,
    /* 0x4C */ KEY_PAD_5,
    /* 0x4D */ KEY_PAD_6,
    /* 0x4E */ KEY_PAD_PLUS,
    /* 0x4F */ KEY_PAD_1,
    /* 0x50 */ KEY_PAD_2,
    /* 0x51 */ KEY_PAD_3,
    /* 0x52 */ KEY_PAD_0,
    /* 0x53 */ KEY_PAD_POINT,
    /* 0x54 */ KEY_54,
    /* 0x55 */ KEY_55,
    /* 0x56 */ KEY_56,
    /* 0x57 */ KEY_F11,
    /* 0x58 */ KEY_F12,
    /* 0x59 */ KEY_MAX,
} keymap_idx_t;

static char keymap[][4] = {
    /* 扫描码 未按 shift 的 ASCII, 按下 shift 的 ASCII */
    /* ---------------------------------- */
    /* 0x00 */ {INV, INV},    // NULL
    /* 0x01 */ {0x1b, 0x1b},  // ESC
    /* 0x02 */ {'1', '!'},
    /* 0x03 */ {'2', '@'},
    /* 0x04 */ {'3', '#'},
    /* 0x05 */ {'4', '$'},
    /* 0x06 */ {'5', '%'},
    /* 0x07 */ {'6', '^'},
    /* 0x08 */ {'7', '&'},
    /* 0x09 */ {'8', '*'},
    /* 0x0A */ {'9', '('},
    /* 0x0B */ {'0', ')'},
    /* 0x0C */ {'-', '_'},
    /* 0x0D */ {'=', '+'},
    /* 0x0E */ {'\b', '\b'},  // BACKSPACE
    /* 0x0F */ {'\t', '\t'},  // TAB
    /* 0x10 */ {'q', 'Q'},
    /* 0x11 */ {'w', 'W'},
    /* 0x12 */ {'e', 'E'},
    /* 0x13 */ {'r', 'R'},
    /* 0x14 */ {'t', 'T'},
    /* 0x15 */ {'y', 'Y'},
    /* 0x16 */ {'u', 'U'},
    /* 0x17 */ {'i', 'I'},
    /* 0x18 */ {'o', 'O'},
    /* 0x19 */ {'p', 'P'},
    /* 0x1A */ {'[', '{'},
    /* 0x1B */ {']', '}'},
    /* 0x1C */ {'\n', '\n'},  // ENTER
    /* 0x1D */ {INV, INV},    // CTRL_L
    /* 0x1E */ {'a', 'A'},
    /* 0x1F */ {'s', 'S'},
    /* 0x20 */ {'d', 'D'},
    /* 0x21 */ {'f', 'F'},
    /* 0x22 */ {'g', 'G'},
    /* 0x23 */ {'h', 'H'},
    /* 0x24 */ {'j', 'J'},
    /* 0x25 */ {'k', 'K'},
    /* 0x26 */ {'l', 'L'},
    /* 0x27 */ {';', ':'},
    /* 0x28 */ {'\'', '"'},
    /* 0x29 */ {'`', '~'},
    /* 0x2A */ {INV, INV},    // SHIFT_L
    /* 0x2B */ {'\\', '|'},
    /* 0x2C */ {'z', 'Z'},
    /* 0x2D */ {'x', 'X'},
    /* 0x2E */ {'c', 'C'},
    /* 0x2F */ {'v', 'V'},
    /* 0x30 */ {'b', 'B'},
    /* 0x31 */ {'n', 'N'},
    /* 0x32 */ {'m', 'M'},
    /* 0x33 */ {',', '<'},
    /* 0x34 */ {'.', '>'},
    /* 0x35 */ {'/', '?'},
    /* 0x36 */ {INV, INV},    // SHIFT_R
    /* 0x37 */ {'*', '*'},    // PAD *
    /* 0x38 */ {INV, INV},    // ALT_L
    /* 0x39 */ {' ', ' '},    // SPACE
    /* 0x3A */ {INV, INV},    // CAPSLOCK
    /* 0x3B */ {INV, INV},    // F1
    /* 0x3C */ {INV, INV},    // F2
    /* 0x3D */ {INV, INV},    // F3
    /* 0x3E */ {INV, INV},    // F4
    /* 0x3F */ {INV, INV},    // F5
    /* 0x40 */ {INV, INV},    // F6
    /* 0x41 */ {INV, INV},    // F7
    /* 0x42 */ {INV, INV},    // F8
    /* 0x43 */ {INV, INV},    // F9
    /* 0x44 */ {INV, INV},    // F10
    /* 0x45 */ {INV, INV},    // NUMLOCK
    /* 0x46 */ {INV, INV},    // SCRLOCK
    /* 0x47 */ {'7', INV},    // pad 7 - Home
    /* 0x48 */ {'8', INV},    // pad 8 - Up
    /* 0x49 */ {'9', INV},    // pad 9 - PageUp
    /* 0x4A */ {'-', '-'},    // pad -
    /* 0x4B */ {'4', INV},    // pad 4 - Left
    /* 0x4C */ {'5', INV},    // pad 5
    /* 0x4D */ {'6', INV},    // pad 6 - Right
    /* 0x4E */ {'+', '+'},    // pad +
    /* 0x4F */ {'1', INV},    // pad 1 - End
    /* 0x50 */ {'2', INV},    // pad 2 - Down
    /* 0x51 */ {'3', INV},    // pad 3 - PageDown
    /* 0x52 */ {'0', INV},    // pad 0 - Insert
    /* 0x53 */ {'.', 0x7f},   // pad . - Delete
    /* 0x54 */ {INV, INV},    //
    /* 0x55 */ {INV, INV},    //
    /* 0x56 */ {INV, INV},    //
    /* 0x57 */ {INV, INV},    // F11
    /* 0x58 */ {INV, INV},    // F12
    /* 0x59 */ {INV, INV},    // 自定义的最大扫描码
};

static bool ctrl_state;
static bool alt_state;
static bool shift_state;
static bool capslock_state; // 大写锁定
static bool scrlock_state;  // 滚动锁定
static bool numlock_state;  // 数字锁定
static bool extcode_state;  // e0 标记

void keyboard_interrupt_handler(void)
{
    uchar is_ext = 0;  // 按下的是扩展码

    send_eoi(0x21);

    // 读取扫描码，不读取 8024 不会响应接下来的键盘操作
    uchar scancode = in_byte(0x60);

    // 扩展码头
    if (scancode == 0xe0) {
        extcode_state = true;
        return;
    }

    // 继续接收扩展码
    if (extcode_state) {
        is_ext = 1;
        scancode |= 0xe000;  // 补全扫描码
        extcode_state = false;  // 清 e0 标记
    }

    ushort makecode = (scancode & 0x007f);
    if (makecode > KEY_MAX) {
        return;  // 不支持的按键
    }

    // 是否是断码
    bool breakcode = ((scancode & 0x0080) != 0);
    if (breakcode) {
        if (makecode == KEY_CTRL_L) {
            ctrl_state = false;
        } else if (makecode == KEY_SHIFT_L || makecode == KEY_SHIFT_R) {
            shift_state = false;
        } else if (makecode == KEY_ALT_L) {
            alt_state = false;
        }
        return;
    }

    // 如果是通码
    if (makecode == KEY_NUMLOCK) {
        numlock_state = !numlock_state;
    } else if (makecode == KEY_CAPSLOCK) {
        capslock_state = !capslock_state;
    } else if (makecode == KEY_SCRLOCK) {
        scrlock_state = !scrlock_state;
    } else if (makecode == KEY_CTRL_L) {
        ctrl_state = true;
    } else if (makecode == KEY_SHIFT_L || makecode == KEY_SHIFT_R) {
        shift_state = true;
    } else if (makecode == KEY_ALT_L) {
        alt_state = true;
    }

    // 计算 shift 状态
    bool shift = shift_state;
    if (capslock_state) {
        shift = !shift;
    }

    // 获取按键 ASCII 码
    char ch = INV;
    if (is_ext && (makecode != KEY_SLASH)) {
        ch = keymap[makecode][1];  // 扩展码字符跟按下 shift 的普通码一样
    } else if (is_ext && (makecode == KEY_SLASH)) {
        ch = keymap[makecode][0];  // e035 跟不按 shift 的普通码一样
    } else {
        ch = keymap[makecode][shift];  // 普通码
    }

    if (ch == INV) {
        return;  // 不可见字符直接返回
    }

    printk("%c", ch);  // 打印可见字符
    return;
}

