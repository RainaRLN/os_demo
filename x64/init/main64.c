void main_x64(void)
{
    __asm__("xchg bx, bx");
    __asm__("xchg bx, bx");
    while(1);
}
