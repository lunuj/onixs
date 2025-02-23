#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <onixs/types.h>

#define IDT_SIZE 256
#define ENTRY_SIZE 0x20
typedef struct gate_t
{
    uint16 offset0;
    uint16 selector;
    uint8 reserved;
    uint8 type:4;
    uint8 segmeny:1;
    uint8 DPL:2;
    uint8 present:1;
    uint16 offset1;
}_packed gate_t;

void interrupt_init();
typedef void *handler_t; // 中断处理函数

#endif // INTERRUPT_H