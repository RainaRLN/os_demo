[section .text]
[bits 64]

extern clock_interrupt_handler
extern keyboard_interrupt_handler

global clock_handler_entry
clock_handler_entry:
    call clock_interrupt_handler
    iretq

global keyboard_handler_entry
keyboard_handler_entry:
    call keyboard_interrupt_handler
    iretq

