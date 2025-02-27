#include <onixs/stdio.h>
#include <onixs/stdarg.h>
#include <onixs/debug.h>
static char buf[1024];
void debugk(char *file, int line, const char *fmt, ...){
    va_list args;
    va_start(args, fmt);
    int i = vsprintf(buf, fmt, args);
    va_end(args);
    printk("[DEBUG]: [%s] [%d] %s", file, line, buf);
}