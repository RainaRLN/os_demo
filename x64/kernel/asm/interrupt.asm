[section .text]
[bits 64]

extern clock_interrupt_handler
extern keyboard_interrupt_handler
extern exception_interrupt_handler

global clock_handler_entry
clock_handler_entry:
    call clock_interrupt_handler
    iretq

global keyboard_handler_entry
keyboard_handler_entry:
    call keyboard_interrupt_handler
    iretq

; Vol.3A 6-13 Figure 6-4
%macro NO_ERRCODE_ENTRY 1
global interrupt_handler_entry_%1
interrupt_handler_entry_%1:
    ; EFLAGS
    ; CS
    ; EIP  <--- ESP
    mov r9, 0x00  ; fake error code
    mov r8, [rsp + 8 * 2]  ; eflags
    mov rcx, [rsp + 8]  ; cs
    mov rdx, [rsp]  ; eip
    mov rsi, rsp  ; esp
    mov rdi, %1  ; idt_index
    call exception_interrupt_handler
    iretq
%endmacro

%macro DO_ERRCODE_ENTRY 1
global interrupt_handler_entry_%1
interrupt_handler_entry_%1:
    ; EFLAGS
    ; CS
    ; EIP
    ; Error Code <--- ESP
    mov r9, [rsp]  ; error code
    mov r8, [rsp + 8 * 3]  ; eflags
    mov rcx, [rsp + 8 * 2]  ; cs
    mov rdx, [rsp + 8]  ; eip
    mov rsi, rsp  ; esp
    mov rdi, %1  ; idt_index
    call exception_interrupt_handler
    iretq
%endmacro

NO_ERRCODE_ENTRY 0x00  ; Divide Error
NO_ERRCODE_ENTRY 0x01  ; Debug Exception
NO_ERRCODE_ENTRY 0x02  ; NMI Interrupt
NO_ERRCODE_ENTRY 0x03  ; Breakpoint
NO_ERRCODE_ENTRY 0x04  ; Overflow
NO_ERRCODE_ENTRY 0x05  ; BOUND Range Exceeded
NO_ERRCODE_ENTRY 0x06  ; Invalid Opcode (Undefined Opcode)
NO_ERRCODE_ENTRY 0x07  ; Device Not Available (No Math Coprocessor)
DO_ERRCODE_ENTRY 0x08  ; Double Fault
NO_ERRCODE_ENTRY 0x09  ; Coprocessor Segment Overrun (reserved)
DO_ERRCODE_ENTRY 0x0a  ; Invalid TSS
DO_ERRCODE_ENTRY 0x0b  ; Segment Not Present
DO_ERRCODE_ENTRY 0x0c  ; Stack Segment Fault
DO_ERRCODE_ENTRY 0x0d  ; General Protection
DO_ERRCODE_ENTRY 0x0e  ; Page Fault
NO_ERRCODE_ENTRY 0x0f  ; Intel reserved
NO_ERRCODE_ENTRY 0x10  ; x87 FPU Floating-Point Error (Math Fault)
DO_ERRCODE_ENTRY 0x11  ; Alignment Check
NO_ERRCODE_ENTRY 0x12  ; Machine Check
NO_ERRCODE_ENTRY 0x13  ; SIMD Floating-Point Exception
NO_ERRCODE_ENTRY 0x14  ; Virtualization Exception
DO_ERRCODE_ENTRY 0x15  ; Control Protection Exception

NO_ERRCODE_ENTRY 0x16  ; Reserved
NO_ERRCODE_ENTRY 0x17  ; Reserved
NO_ERRCODE_ENTRY 0x18  ; Reserved
NO_ERRCODE_ENTRY 0x19  ; Reserved
NO_ERRCODE_ENTRY 0x1a  ; Reserved
NO_ERRCODE_ENTRY 0x1b  ; Reserved
NO_ERRCODE_ENTRY 0x1c  ; Reserved
NO_ERRCODE_ENTRY 0x1d  ; Reserved
NO_ERRCODE_ENTRY 0x1e  ; Reserved
NO_ERRCODE_ENTRY 0x1f  ; Reserved

global interrupt_handler_table
interrupt_handler_table:
    dq interrupt_handler_entry_0x00
    dq interrupt_handler_entry_0x01
    dq interrupt_handler_entry_0x02
    dq interrupt_handler_entry_0x03
    dq interrupt_handler_entry_0x04
    dq interrupt_handler_entry_0x05
    dq interrupt_handler_entry_0x06
    dq interrupt_handler_entry_0x07
    dq interrupt_handler_entry_0x08
    dq interrupt_handler_entry_0x09
    dq interrupt_handler_entry_0x0a
    dq interrupt_handler_entry_0x0b
    dq interrupt_handler_entry_0x0c
    dq interrupt_handler_entry_0x0d
    dq interrupt_handler_entry_0x0e
    dq interrupt_handler_entry_0x0f
    dq interrupt_handler_entry_0x10
    dq interrupt_handler_entry_0x11
    dq interrupt_handler_entry_0x12
    dq interrupt_handler_entry_0x13
    dq interrupt_handler_entry_0x14
    dq interrupt_handler_entry_0x15
    dq interrupt_handler_entry_0x16
    dq interrupt_handler_entry_0x17
    dq interrupt_handler_entry_0x18
    dq interrupt_handler_entry_0x19
    dq interrupt_handler_entry_0x1a
    dq interrupt_handler_entry_0x1b
    dq interrupt_handler_entry_0x1c
    dq interrupt_handler_entry_0x1d
    dq interrupt_handler_entry_0x1e
    dq interrupt_handler_entry_0x1f

