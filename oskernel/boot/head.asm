[ORG  0x1200]

[SECTION .text]
[BITS 32]

global _start
_start:

    xchg bx, bx

stop_cpu:
    hlt
    jmp stop_cpu

