[ORG 0x7c00]  ; MBR 被 BIOS 加载到 0x07C00 处

[SECTION .data]
SETUP_ADDR equ 0x500  ; setup 加载到内存中的地址

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
    mov si, msg_load_setup
    call print

    ; 将 setup 读入内存
    mov si, struct_buffer
    mov ah, 0x42
    mov dl, 0x80  ; 0x80 读第一块硬盘
    int 0x13  ; BIOS int 13h/42h
    jc .read_setup_failed

    mov si, msg_jmp_to_setup
    call print

    xchg bx, bx

    jmp SETUP_ADDR  ; 跳进 setup

    jmp stop_cpu  ; 不应该返回

.read_setup_failed:
    mov si, msg_load_err
    call print
    jmp stop_cpu

stop_cpu:
    hlt
    jmp stop_cpu

; mov si, str  ; 将字符串首地址传给 si
; call print  ; 调用 print
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

msg_load_setup:
    db "Start loading setup ...", 10, 13, 0  ; \n \r \0
msg_load_err:
    db "Read disk error!", 10, 13, 0
msg_jmp_to_setup:
    db "Jump to setup ...", 10, 13, 0

struct_buffer:
.struct_size: dw 0x10
.sectors: dw 0x02  ; 读多少 sectors
.addr: dd SETUP_ADDR  ; 要读入内存哪个地址
.start_sector: dq 0x01  ; 从哪个 sector 开始读

times 510 - ($ - $$) db 0  ; 补零
db 0x55, 0xaa  ; MBR 结束标记

