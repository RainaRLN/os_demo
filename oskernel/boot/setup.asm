[ORG 0x500]

[SECTION .data]
SEG_BASE equ 0
SEG_LIMIT equ 0xfffff

; 段选择子 -- Vol.3A 3-7 Figure 3-6
CODE_SELECTOR equ (1 << 3)  ; gdt[1], ring 0, 代码段
DATA_SELECTOR equ (2 << 3)  ; gdt[2], ring 0, 数据段

; 编辑 GDT 表 -- Vol.3A 3-10 Figure 3-8
gdt_base:
    dd 0, 0  ; 处理器不会使用第一个 GDT 项 -- Vol.3A 3-15
gdt_code:
    dw SEG_LIMIT & 0xffff  ; Segment Limint 15:00
    dw SEG_BASE & 0xffff  ; Base Address 15:00
    db SEG_BASE >> 16 & 0xff  ; Base 23:16
    ; Segment present
    ; Descriptor privilege level = 0
    ; Descriptor type = 1, code or data
    ; Segment type = 1000b, code, excute only -- Vol.3A 3-12 Table 3-1
    ;    P_DPL_S_TYPE
    db 0b1_00_1_1000
    ; Granularity = 1, 段长以 4K 为单位
    ; DB = 1, default operation size is 32bit
    ; AVL reserved
    ; Seg Limit 19:16
    ;    G_DB_AVL_LIMIT
    db 0b1_1_00_0000 | (SEG_LIMIT >> 16 & 0xf)
    db SEG_BASE >> 24 & 0xff  ; Base 31:24
gdt_data:
    dw SEG_LIMIT & 0xffff
    dw SEG_BASE & 0xffff
    db SEG_BASE >> 16 & 0xff
    ; Segment present = 1
    ; Descriptor privilege level = 0
    ; Descriptor type = 1, code or data
    ; Segment type = 0010b, data, read/write
    ;    P_DPL_S_TYPE
    db 0b1_00_1_0010
    ; Granularity = 1, 段长以 4K 为单位
    ;    G_DB_AVL_LIMIT
    db 0b1_1_00_0000 | (SEG_LIMIT >> 16 & 0xf)
    db SEG_BASE >> 24 & 0xff
gdt_ptr:  ; Vol.3A 2-12 2.4.1
    dw $ - gdt_base  ; 全局描述符表大小
    dd gdt_base  ; 全局描述符表基地址

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
    mov si, msg_prepare_protected
    call print

enter_protected_mode:
    cli  ; 关中断

    lgdt [gdt_ptr]  ; 加载 gdt 表

    ; 开 A20
    in al, 92h
    or al, 00000010b
    out 92h, al

    ; 开启保护模式
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp CODE_SELECTOR:protected_start  ; 跳到保护模式，刷新影子寄存器

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

[BITS 32]  ; 保护模式下为 32 位
protected_start:
    ; 初始化所有段选择子
    mov ax, DATA_SELECTOR
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov esp, 0x9fbff  ; 设置栈

    xchg bx, bx

stop_cpu:
    hlt
    jmp stop_cpu

msg_prepare_protected:
    db "Prepare to go into protected mode ...", 10, 13, 0  ; \n \r \0

