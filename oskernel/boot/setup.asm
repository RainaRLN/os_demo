[ORG 0x500]

[SECTION .data]
KERNEL_ADDR equ 0x1200
X64_KERNEL_ADDR equ 0x100000

E820_ARDS_NUM dw 0
E820_ARDS_NUM_ADDR equ 0x7e00
E820_ARDS_ADDR equ 0x7e02

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

get_mem_map:
    xor ebx, ebx
    mov di, E820_ARDS_ADDR
.next:
    mov eax, 0xe820  ; 子功能号
    mov ecx, 20  ; ARDS 结构体大小
    mov edx, 0x534d4150  ; MAGIC
    int 0x15  ; BIOS 中断号

    jc .get_map_err  ; CF = 1, 出错

    inc dword [E820_ARDS_NUM]  ; 记录结构体数量
    add di, cx  ; 下一个填充位置

    cmp ebx, 0  ; ebx == 0, 表示是最后一个 ARDS 结构体，退出
    jne .next  ; 继续下一个

    mov ax, [E820_ARDS_NUM]
    mov [E820_ARDS_NUM_ADDR], ax  ; 保存结构体总数

    mov si, msg_get_mem_map_ok
    call print
    jmp enter_protected_mode

.get_map_err:
    mov si, msg_get_mem_map_err
    call print
    jmp $

enter_protected_mode:
    mov si, msg_prepare_protected
    call print

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

    mov esp, 0x7b00  ; 设置栈

    xchg bx, bx

    ; 将内核读入内存
    ; 保护模式下无法使用 BIOS 中断
    mov edi, KERNEL_ADDR  ; 读到内存哪个位置
    mov ecx, 3  ; 从哪个扇区开始读
    mov bl, 30  ; 读多少个扇区
    call read_hd

    ; 将 x64 内核读入内存
    mov edi, X64_KERNEL_ADDR
    mov ecx, 40
    mov bl, 255
    call read_hd

    ; 跳到内核
    jmp CODE_SELECTOR:KERNEL_ADDR

stop_cpu:
    hlt
    jmp stop_cpu

read_hd:
    ; 保存寄存器
    push eax
    push ebx
    push ecx
    push edx

    ; 0x1f2 指定读取或写入的扇区数
    mov dx, 0x1f2
    mov al, bl
    out dx, al

    ; 0x1f3 LBA 地址 7:0
    inc dx
    mov al, cl
    out dx, al

    ; 0x1f4 LBA 地址 15:8
    inc dx
    mov al, ch
    out dx, al

    ; 0x1f5 LBA 地址 23:16
    inc dx
    shr ecx, 16
    mov al, cl
    out dx, al

    ; 0x1f6
    ; bit[3:0]: LBA 地址 27:24
    ; bit[4]: 0->主盘, 1->从盘
    ; bit[5] bit[7]: 固定为 1
    ; bit[6]: 0->CHS模式, 1->LBA模式
    inc dx
    shr ecx, 8
    and cl, 0x0f
    mov al, 0b1110_0000
    or al, cl
    out dx, al

    ; 0x1f7 控制/状态寄存器
    inc dx
    mov al, 0x20  ; 请求读
    out dx, al

    ; 开始通过 0x1f0 读数据
    movzx cx, bl  ; 读取多少扇区，将 bl 内容扩展到 cx 中，高位补零
.start_read:
    push cx  ; 后面会用到 cx，保存
    call .waits
    call .read_one_sector
    pop cx  ; 恢复 loop 次数
    loop .start_read

    ; 恢复寄存器
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret  ; 返回

    ; 不断查询 0x1f7，直到硬盘不忙且准备好数据传输
.waits:
    mov dx, 0x1f7
    in al, dx
    and al, 0x88  ; 保留 BSY 和 DRQ，其他位清零
    cmp al, 0x08  ; BSY=0 不忙, DRQ=1 准备好数据传输
    jnz .waits
    ret

.read_one_sector:
    mov cx, 256  ; 一个扇区 256 个字
    mov dx, 0x1f0
.readw:
    in ax, dx
    mov [edi], ax
    add edi, 2
    loop .readw
    ret

msg_prepare_protected:
    db "Prepare to go into protected mode ...", 10, 13, 0  ; \n \r \0
msg_get_mem_map_ok:
    db "Query memory map ok.", 10, 13, 0
msg_get_mem_map_err:
    db "Query memory map failed.", 10, 13, 0
