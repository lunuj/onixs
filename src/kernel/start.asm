[bits 32]

extern console_init
extern gdt_init
extern memory_init
extern kernel_init

global _start
section .start
_start:
    push ebx; ards_count
    push eax; magic
    call console_init
    call gdt_init
    call memory_init
    call kernel_init
    ; int 0x80
    ; mov bx ,0
    ; div bx
    jmp $