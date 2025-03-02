#ifndef RTC_H
#define RTC_H

#include <onixs/types.h>

#define CMOS_ADDR 0x70 // CMOS 地址寄存器
#define CMOS_DATA 0x71 // CMOS 数据寄存器

#define CMOS_A 0x0a
#define CMOS_B 0x0b
#define CMOS_C 0x0c
#define CMOS_D 0x0d
#define CMOS_NMI 0x80

// 下面是 CMOS 信息的寄存器索引
#define CMOS_SECOND_READ 0x00  // (0 ~ 59)
#define CMOS_MINUTE_READ 0x02  // (0 ~ 59)
#define CMOS_HOUR_READ 0x04    // (0 ~ 23)
#define CMOS_WEEKDAY_READ 0x06 // (1 ~ 7) 星期天 = 1，星期六 = 7
#define CMOS_DAY_READ 0x07     // (1 ~ 31)
#define CMOS_MONTH_READ 0x08   // (1 ~ 12)
#define CMOS_YEAR_READ 0x09    // (0 ~ 99)
#define CMOS_CENTURY_READ 0x32 // 可能不存在
#define CMOS_NMI_READ 0x80

#define CMOS_SECOND_WRITE 0x01
#define CMOS_MINUTE_WRITE 0x03
#define CMOS_HOUR_WRITE 0x05

uint8 cmos_read(uint8 addr);
void cmos_write(uint8 addr, uint8 value);
void rtc_init();
void set_alarm(uint32 secs);
#endif // RTC_H