[ORG 0x7c00]  ; MBR 被 BIOS 加载到 0x07C00 处

[SECTION .text]
[BITS 16]  ; 当前 CPU 处于实模式，为 16 位

global _start
_start:
    ; 设置屏幕位文本模式，清屏
    mov ax, 0x03  
    int 0x10  ; BIOS 10h 中断

    xchg bx, bx  ; 断点

    ; 初始化段寄存器
    xor ax, ax  ; ax 清零
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; 调用 print
    ; 用 si 传字符串首地址
    xor si, si
    mov si, msg
    call print

    jmp $

print:
    mov ah, 0x0e  ; 显示一个字符，且光标移动
    mov bh, 0x00  ; 页号
    mov bl, 0x00  ; 文字模式下 BL 无作用
.loop:
    mov al, [si]  ; 要显示的字符
    cmp al, 0  ; 字符串结束符
    jz .done
    int 0x10  ; BIOS 10h

    inc si  ; 下一个字符
    jmp .loop
.done:
    ret

msg:
    db "hello world", 10, 13, 0  ; \n \r \0

times 510 - ($ - $$) db 0  ; 补零
db 0x55, 0xaa  ; MBR 结束标记

