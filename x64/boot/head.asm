[SECTION .text]
[bits 64]

extern main_x64

global _start
_start:
    xchg bx, bx
    call main_x64
    jmp $

