#include <onixs/stdio.h>
#include <onixs/stdarg.h>
#include <onixs/debug.h>
#include <onixs/printk.h>
#include <onixs/device.h>
static char buf[1024];
char *log_str[] = 
{
    "FATAL",
    "ERROR",
    "WARN",
    "INFO",
    "DEBUG",
    "TRACE",
    "NONE"
};

void debugk(char *file, int line, const char *fmt, ...){
    device_t *device = device_find(DEV_SERIAL, 0);
    if(!device)
    {
        device  = device_find(DEV_CONSOLE, 0);
    }
    int i = sprintf(buf, "[KERNEL]: [%s] [%d] ", file, line);
    device_write(device->dev, buf, i, 0, 0);
    va_list args;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    device_write(device->dev, buf, i, 0, 0);
}

void logk(int lv, const char *fmt, ...)
{
    device_t *device = device_find(DEV_SERIAL, 0);
    if(!device)
    {
        device  = device_find(DEV_CONSOLE, 0);
    }
    int i = sprintf(buf, "[%s]: ", log_str[lv]);
    device_write(device->dev, buf, i, 0, 0);
    va_list args;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    if(lv < 0 || lv > LOG_LV){
        lv = LOG_LV + 1;
    }
    device_write(device->dev, buf, i, 0, 0);
}