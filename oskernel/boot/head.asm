[SECTION .text]
[BITS 32]

extern kernel_main

global _start
_start:

    xchg bx, bx

    call kernel_main

stop_cpu:
    hlt
    jmp stop_cpu

