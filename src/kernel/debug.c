#include <onixs/stdio.h>
#include <onixs/stdarg.h>
#include <onixs/debug.h>
#include <onixs/printk.h>
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
    va_list args;
    va_start(args, fmt);
    int i = vsprintf(buf, fmt, args);
    va_end(args);
    printk("[KERNEL]: [%s] [%d] %s", file, line, buf);
}

void logk(int lv, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int i = vsprintf(buf, fmt, args);
    va_end(args);
    if(lv < 0 || lv > LOG_LV){
        lv = LOG_LV + 1;
    }
    printk("[%s]: %s", log_str[lv], buf);
}