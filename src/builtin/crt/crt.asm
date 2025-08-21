[bits 32]

section .text
global _start

extern __libc_start_main
extern _init
extern _fini
extern main

_start:
    xor ebx, ebx; 清除栈底
    pop esi; 获取栈顶参数 argc
    mov ecx, esp; 获取栈顶参数 argv

    and esp, -16; 栈对齐，SSE需要16字节对齐
    push eax;
    push esp; 用户程序栈最大地址
    push edx; 动态链接器
    push _init; libc 构造函数
    push _fini; libc 析构函数
    push ecx; argv
    push esi; argc
    push main; 主函数

    call __libc_start_main

    ud2; 无法到达