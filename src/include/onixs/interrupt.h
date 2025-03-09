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

#define IRQ_CLOCK 0      // 时钟
#define IRQ_KEYBOARD 1   // 键盘
#define IRQ_CASCADE 2    // 8259 从片控制器
#define IRQ_SERIAL_2 3   // 串口 2
#define IRQ_SERIAL_1 4   // 串口 1
#define IRQ_PARALLEL_2 5 // 并口 2
#define IRQ_FLOPPY 6     // 软盘控制器
#define IRQ_PARALLEL_1 7 // 并口 1
#define IRQ_RTC 8        // 实时时钟
#define IRQ_REDIRECT 9   // 重定向 IRQ2
#define IRQ_MOUSE 12     // 鼠标
#define IRQ_MATH 13      // 协处理器 x87
#define IRQ_HARDDISK 14  // ATA 硬盘第一通道
#define IRQ_HARDDISK2 15 // ATA 硬盘第二通道

#define IRQ_MASTER_NR 0x20 // 主片起始向量号
#define IRQ_SLAVE_NR 0x28  // 从片起始向量号

typedef void *handler_t; // 中断处理函数

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
void interrupt_register(uint32 irq, handler_t handler);
void interrupt_mask(uint32 irq, bool enable);
void send_eoi(int vector);

#endif // INTERRUPT_H

