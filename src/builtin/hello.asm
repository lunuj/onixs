[bits 32]

global _start
section .text
_start:
    ; write(stdout, message , sizeof(message))
    mov eax, 4; write
    mov ebx, 1; stdout
    mov ecx, message
    mov edx, message.end - message - 1
    int 0x80

    ; exit(0);
    mov eax, 1;
    mov ebx, 0;
    int 0x80

section .data
message:
    db "hello onix!!!", 10, 0
.end:

section .bss

buffer: resb 1024; 预留 1024 个字节的空间

; nasm -f elf32 hello.asm -o hello.o
; x86_64-elf-ld -m elf_i386 -static hello.o -o hello.out -Ttext 0x1001000
; x86_64-elf-ld -T hello.ld -m elf_i386 -static hello.o -o hello.out