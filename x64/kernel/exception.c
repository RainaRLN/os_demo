#include "linux/kernel.h"
#include "linux/types.h"

// Vol.3A 6-2 Table 6-1
char *messages[] = {
    "#DE Divide Error",
    "#DB Debug Exception",
    "#-- NMI Interrupt",
    "#BP Breakpoint",
    "#OF Overflow",
    "#BR BOUND Range Exceeded",
    "#UD Invalid Opcode (Undefined Opcode)",
    "#NM Device Not Available (No Math Coprocessor)",
    "#DF Double Fault",
    "#MF Coprocessor Segment Overrun (reserved)",
    "#TS Invalid TSS",
    "#NP Segment Not Present",
    "#SS Stack Segment Fault",
    "#GP General Protection",
    "#PF Page Fault",
    "#-- (Intel reserved. Do not use)",
    "#MF x87 FPU Floating-Point Error (Math Fault)",
    "#AC Alignment Check",
    "#MC Machine Check",
    "#XM SIMD Floating-Point Exception",
    "#VE Virtualization Exception",
    "#CP Control Protection Exception",
    "#-- Reserved",  // 0x16~0x1F
};

void exception_interrupt_handler(int64 idt_index, int64 esp, int64 eip, 
                            char cs, int64 eflags, int64 err_code)
{
    printk("\n=====================================\n");
    printk("    VECTOR: 0x%2x\n", idt_index);
    printk(" EXCEPTIOM: %s\n", messages[idt_index > 0x16 ? 0x16 : idt_index]);
    printk("ERROR CODE: 0x%8x\n", err_code);
    printk("    EFLAGS: 0x%8x\n", eflags);
    printk("        CS: 0x%4x\n", cs);
    printk("       EIP: 0x%8x\n", eip);
    printk("       ESP: 0x%8x\n", esp);
    printk("=====================================\n");

    while (1);
    return;
}

