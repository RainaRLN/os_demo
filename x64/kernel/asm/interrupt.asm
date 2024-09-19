[section .text]
[bits 64]

extern clock_interrupt_handler

global clock_handler_entry
clock_handler_entry:
    call clock_interrupt_handler
    iretq

