#include <onixs/interrupt.h>
#include <onixs/global.h>
#include <onixs/debug.h>

gate_t idt[IDT_SIZE];
pointer_t idt_ptr;

extern void interrupt_handler();

void interrupt_init(){
    for(int i = 0; i < IDT_SIZE; i++){
        gate_t *gate = &idt[i];
        gate->offset0 = (uint32)interrupt_handler&0xFFFF;
        gate->offset1 = ((uint32)interrupt_handler>>16)&0xFFFF;
        gate->selector = 1<<3;
        gate->reserved = 0;
        gate->type = 0b1110;
        gate->segmeny = 0;
        gate->DPL = 0;
        gate->present = 1;
    }
    idt_ptr.base = (uint32)idt;
    idt_ptr.limit = sizeof(idt) - 1;
    asm volatile("lidt idt_ptr\n");
}