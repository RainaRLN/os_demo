#define LOGO_H 6
#define LOGO_W 42

void show(int x, int y, char color, char c)
{
    // 0xb8000 显存地址
    char* p_strdst = (char*)(0xb8000 + x * 2 + y * 80 * 2);

    *p_strdst = c;
    *(p_strdst+1) = color;
    return;
}

void kernel_main(void)
{
    int x = 0;
    int y = 0;
    int i = 1;
    const char logo[LOGO_H][LOGO_W] = {
        " _      _                    ____   _____ ",
        "| |    (_)                  / __ \\ / ____|",
        "| |     _ _ __   __ _ ___  | |  | | (___  ",
        "| |    | | '_ \\ / _` / __| | |  | |\\___ \\ ",
        "| |____| | | | | (_| \\__ \\ | |__| |____) |",
        "|______|_|_| |_|\\__,_|___/  \\____/|_____/ ",
    };

    for (y = 0; y < LOGO_H; ++y) {
        for (x = 0; x < LOGO_W; ++x) {
            show(x+18, y+7, (char)((i+y)%6+1), logo[y][x]);
            ++i;
        }
    }

    return;
}

