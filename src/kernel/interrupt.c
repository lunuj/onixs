#include <onixs/interrupt.h>
#include <onixs/global.h>
#include <onixs/debug.h>
#include <onixs/stdio.h>

gate_t idt[IDT_SIZE];
pointer_t idt_ptr;

handler_t handler_table[IDT_SIZE];
extern handler_t handler_entry_table[ENTRY_SIZE];

static char *messages[] = {
    "#DE Divide Error\0",
    "#DB RESERVED\0",
    "--  NMI Interrupt\0",
    "#BP Breakpoint\0",
    "#OF Overflow\0",
    "#BR BOUND Range Exceeded\0",
    "#UD Invalid Opcode (Undefined Opcode)\0",
    "#NM Device Not Available (No Math Coprocessor)\0",
    "#DF Double Fault\0",
    "    Coprocessor Segment Overrun (reserved)\0",
    "#TS Invalid TSS\0",
    "#NP Segment Not Present\0",
    "#SS Stack-Segment Fault\0",
    "#GP General Protection\0",
    "#PF Page Fault\0",
    "--  (Intel reserved. Do not use.)\0",
    "#MF x87 FPU Floating-Point Error (Math Fault)\0",
    "#AC Alignment Check\0",
    "#MC Machine Check\0",
    "#XF SIMD Floating-Point Exception\0",
    "#VE Virtualization Exception\0",
    "#CP Control Protection Exception\0",
};

void exception_handler(int vector)
{
    char *message = NULL;
    if (vector < 22)
    {
        message = messages[vector];
    }
    else
    {
        message = messages[15];
    }
    printk("[ERROR]: [%#02X] %s \n", vector, messages[vector]);
    // 阻塞
    while (true)
        ;
}

void interrupt_init(){
    //初始化异常处理函数
    for(int i = 0; i < ENTRY_SIZE; i++){
        gate_t *gate = &idt[i];
        handler_t handler = handler_entry_table[i];
        gate->offset0 = (uint32)handler&0xFFFF;
        gate->offset1 = ((uint32)handler>>16)&0xFFFF;
        gate->selector = 1<<3;
        gate->reserved = 0;
        gate->type = 0b1110;
        gate->segmeny = 0;
        gate->DPL = 0;
        gate->present = 1;
    }

    for(int size = 0; size < ENTRY_SIZE; size++){
        handler_table[size] = exception_handler;
    }
    
    idt_ptr.base = (uint32)idt;
    idt_ptr.limit = sizeof(idt) - 1;
    asm volatile("lidt idt_ptr\n");
}