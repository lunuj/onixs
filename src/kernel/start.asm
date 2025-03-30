[bits 32]

magic   equ 0xE85250D6
i386    equ 0
length  equ header_end - header_start

section .multiboot2
header_start:
    dd magic
    dd i386
    dd length
    dd -(magic + i386 + length)

    dw 0
    dw 8
    dd 8
header_end:

code_selector equ (1 << 3)
data_selector equ (2 << 3)

extern gdt_ptr
extern console_init
extern gdt_init
extern memory_init
extern kernel_init

global _start
section .start
_start:
    push ebx; ards_count
    push eax; magic
    call console_init   ; 初始化控制台
    call gdt_init       ; 全局描述符初始化
    lgdt [gdt_ptr]
    jmp dword code_selector:_next
_next:
    ; 初始化段寄存器
    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ; 内存初始化
    call memory_init
    ; 修改栈顶
    mov esp, 0x10000
    ; 内核初始化
    call kernel_init
    ; int 0x80
    ; mov bx ,0
    ; div bx
    jmp $