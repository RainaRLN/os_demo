[ORG 0x500]

[SECTION .text]
[BITS 16]

global _start
_start:
    ; 初始化段寄存器
    xor ax, ax  ; ax 清零
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    xor si, si
    mov si, msg_hello
    call print

stop_cpu:
    hlt
    jmp stop_cpu

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

msg_hello:
    db "Hello setup", 10, 13, 0  ; \n \r \0

