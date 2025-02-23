[bits 32]

section .text
extern printk
global interrupt_handler
interrupt_handler:
    push message
    call printk
    add esp, 4
    iret

section .data
message:
    db "[INFO]: default interrupt_handler", 10, 0