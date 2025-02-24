#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <onixs/types.h>
#define LOG_INTERRUPT
#if defined(LOG_INTERRUPT)
#define LOGK(fmt, args...) DEBUGK(fmt, ##args)
#else
#define LOGK(fmt, args...)
#endif

#define PIC_M_CTRL 0x20 // 主片的控制端口
#define PIC_M_DATA 0x21 // 主片的数据端口
#define PIC_S_CTRL 0xa0 // 从片的控制端口
#define PIC_S_DATA 0xa1 // 从片的数据端口
#define PIC_EOI 0x20    // 通知中断控制器中断结束

#define IDT_SIZE 256
#define ENTRY_SIZE 0x30
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