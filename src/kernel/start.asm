[bits 32]

extern kernel_init
global _start

_start:
    call kernel_init
    int 0x80
    ; mov bx ,0
    ; div bx
    jmp $