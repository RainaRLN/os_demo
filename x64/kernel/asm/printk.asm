[SECTION .text]
[bits 64]

extern vsprintf
extern strlen
extern console_write

extern printk_buff


; Linux x64 fastcall 调用约定
; func(rdi, rsi, rdx, rcx, r8, r9, stack...)
; 浮点数放 xmm
; 返回值放 rax(整型)和 xmm0(浮点)

; int printk(const char *fmt, ...);
global printk
printk:
    ; 将可变参数入栈
    ; TODO: 不支持 6 个以上的参数
    push r9
    push r8
    push rcx
    push rdx
    push rsi

    ; int vsprintf(char *buf, const char *fmt, va_list args);
    mov rsi, rdi  ; fmt
    mov rdi, printk_buff  ; buf
    mov rdx, rsp  ; args 在栈中的地址
    call vsprintf

    ; void console_write(char *buf, u32 count);
    mov rdi, printk_buff  ; buf
    mov rsi, rax  ; rax 为 vsprintf 的返回值，给 count
    call console_write

    add rsp, 5 * 8  ; 栈还原

    ret

