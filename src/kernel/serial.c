#include <onixs/io.h>
#include <onixs/interrupt.h>
#include <onixs/device.h>
#include <onixs/mutex.h>
#include <onixs/assert.h>
#include <onixs/device.h>
#include <onixs/stdarg.h>
#include <onixs/stdio.h>
#include <onixs/debug.h>
#include <onixs/fifo.h>
#include <onixs/task.h>

#define COM1_IOBASE 0x3F8
#define COM2_IOBASE 0x2F8

#define COM_DATA 0          // 数据寄存器
#define COM_INTR_ENABLE 1   // 中断允许
#define COM_BAUD_LSB 0      // 波特率低字节
#define COM_BAUD_MSB 1      // 波特率高字节
#define COM_INTR_IDENTIFY 2 // 中断识别
#define COM_LINE_CONTROL 3  // 线控制
#define COM_MODEM_CONTROL 4 // 调制解调器控制
#define COM_LINE_STATUS 5   // 线状态
#define COM_MODEM_STATUS 6  // 调制解调器状态

// 线状态
#define LSR_DR 0x1
#define LSR_OE 0x2
#define LSR_PE 0x4
#define LSR_FE 0x8
#define LSR_BI 0x10
#define LSR_THRE 0x20
#define LSR_TEMT 0x40
#define LSR_IE 0x80

#define BUF_LEN 64

typedef struct serial_t
{
    uint16 iobase;           // 端口号基地址
    fifo_t rx_fifo;       // 读 fifo
    char rx_buf[BUF_LEN]; // 读 缓冲
    lock_t rlock;         // 读锁
    task_t *rx_waiter;    // 读等待任务
    lock_t wlock;         // 写锁
    task_t *tx_waiter;    // 写等待任务
} serial_t;

static serial_t serials[2];

void recv_data(serial_t *serial)
{
    char ch = inb(serial->iobase);
    if(ch == '\r')
        ch = '\n';
    
    fifo_put(&serial->rx_fifo, ch);
    if (serial->rx_waiter != NULL)
    {
        task_unblock(serial->rx_waiter);
        serial->rx_waiter = NULL;
    }
    
}

void serial_handler(int vector)
{
    uint32 irq = vector - 0x20;
    assert(irq == IRQ_SERIAL_1 || irq == IRQ_SERIAL_2);
    send_eoi(vector);

    serial_t *serial = &serials[irq - IRQ_SERIAL_1];
    uint8 state = inb(serial->iobase + COM_LINE_STATUS);

    if (state & LSR_DR)
    {
        recv_data(serial);
    }

    if((state & LSR_THRE) && (serial->tx_waiter))
    {
        task_unblock(serial->tx_waiter);
        serial->tx_waiter = NULL;
    }
    
}

int serial_read(serial_t *serial, char *buf, uint32 count)
{
    lock_acquire(&serial->rlock);
    int nr = 0;
    while (nr < count)
    {
        while (fifo_empty(&serial->rx_fifo))
        {
            assert(serial->rx_waiter == NULL);
            serial->rx_waiter = running_task();
            task_block(serial->rx_waiter, NULL, TASK_BLOCKED);
        }
        buf[nr++] = fifo_get(&serial->rx_fifo);
    }
    lock_release(&serial->rlock);
    return nr;
}

int serial_write(serial_t *serial, char *buf, uint32 count)
{
    lock_acquire(&serial->wlock);
    int nr = 0;
    while (nr < count)
    {
        uint8 state = inb(serial->iobase + COM_LINE_STATUS);
        if(state & LSR_THRE)
        {
            outb(serial->iobase, buf[nr++]);
            continue;
        }
        task_t *task = running_task();
        serial->tx_waiter = task;
        task_block(task, NULL, TASK_BLOCKED);
    }
    lock_release(&serial->wlock);
    return nr;
}

void serial_init()
{
    for (size_t i = 0; i < 2; i++)
    {
        serial_t *serial = &serials[i];
        fifo_init(&serial->rx_fifo, serial->rx_buf, BUF_LEN);
        serial->rx_waiter = NULL;
        lock_init(&serial->rlock);

        serial->tx_waiter = NULL;
        lock_init(&serial->wlock);

        uint16 irq;
        if(!i)
        {
            irq = IRQ_SERIAL_1;
            serial->iobase = COM1_IOBASE;
        }
        else
        {
            irq = IRQ_SERIAL_2;
            serial->iobase = COM2_IOBASE;
        }
        
        outb(serial->iobase + COM_LINE_CONTROL, 0x80);
        outb(serial->iobase + COM_BAUD_LSB, 0x30);
        outb(serial->iobase + COM_BAUD_MSB, 0x00);
        outb(serial->iobase + COM_LINE_CONTROL, 0x03);

        outb(serial->iobase + COM_INTR_ENABLE, 0x0d);

        outb(serial->iobase + COM_MODEM_CONTROL, 0b11011);
        outb(serial->iobase, 0xAE);
        if(inb(serial->iobase) != 0xAE)
        {
            continue;
        }
        outb(serial->iobase + COM_MODEM_CONTROL, 0b1011);
        interrupt_register(irq, serial_handler);
        interrupt_mask(irq, true);

        char name[16];
        sprintf(name, "com%d", i + 1);
        device_install(DEV_CHAR, DEV_SERIAL, serial, name, 0, NULL, serial_read, serial_write);
        LOGK("Serial 0x%#x init OK\n", serial->iobase);
    }
}