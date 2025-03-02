#include <onixs/rtc.h>
#include <onixs/time.h>
#include <onixs/interrupt.h>
#include <onixs/debug.h>
#include <onixs/stdlib.h>
#include <onixs/io.h>
#include <onixs/assert.h>

uint8 cmos_read(uint8 addr)
{
    outb(CMOS_ADDR, CMOS_NMI | addr);
    return inb(CMOS_DATA);
};

// 写 cmos 寄存器的值
void cmos_write(uint8 addr, uint8 value)
{
    outb(CMOS_ADDR, CMOS_NMI | addr);
    outb(CMOS_DATA, value);
}

static uint32 volatile counter = 0;

// 实时时钟中断处理函数
void rtc_handler(int vector)
{
    // 实时时钟中断向量号
    assert(vector == 0x28);

    // 向中断控制器发送中断处理完成的信号
    send_eoi(vector);

    // 读 CMOS 寄存器 C，允许 CMOS 继续产生中断
    cmos_read(CMOS_C);

    set_alarm(1);

    LOGK("rtc handler %d...\n", counter++);
    time_init();
}

// 设置 secs 秒后发生实时时钟中断
void set_alarm(uint32 secs)
{
    tm time;
    time_read(&time);

    uint8 sec = secs % 60;
    secs /= 60;
    uint8 min = secs % 60;
    secs /= 60;
    uint32 hour = secs;

    time.tm_sec += sec;
    if (time.tm_sec >= 60)
    {
        time.tm_sec %= 60;
        time.tm_min += 1;
    }

    time.tm_min += min;
    if (time.tm_min >= 60)
    {
        time.tm_min %= 60;
        time.tm_hour += 1;
    }

    time.tm_hour += hour;
    if (time.tm_hour >= 24)
    {
        time.tm_hour %= 24;
    }

    cmos_write(CMOS_HOUR_WRITE, bin_to_bcd(time.tm_hour));
    cmos_write(CMOS_MINUTE_WRITE, bin_to_bcd(time.tm_min));
    cmos_write(CMOS_SECOND_WRITE, bin_to_bcd(time.tm_sec));
}

void rtc_init()
{
    uint8 prev;

    cmos_write(CMOS_B, 0b01000010); // 打开周期中断
    // cmos_write(CMOS_B, 0b00100010); // 打开闹钟中断
    cmos_read(CMOS_C); // 读 C 寄存器，以允许 CMOS 中断

    set_alarm(2);

    // 设置中断频率
    outb(CMOS_A, (inb(CMOS_A) & 0xf) | 0b1110);

    interrupt_register(IRQ_RTC, rtc_handler);
    interrupt_mask(IRQ_RTC, true);
    interrupt_mask(IRQ_CASCADE, true);
}