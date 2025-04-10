#include <onixs/assert.h>
#include <onixs/stdio.h>
#include <onixs/stdarg.h>
#include <onixs/types.h>
#include <onixs/printk.h>
static uint8 buf[1024];
static void spin(char *name){
    printk("\n[ERROR]: spin in %s\n", name);
    while (true);
}

void assert_failure(char *exp, char *file, char *base, int line){
    printk("\n[ERROR]: assert(%s) failed"
                "\n[ERROR]: file: %s"
                "\n[ERROR]: base: %s"
                "\n[ERROR]: line: %d\n", exp, file, base, line);
    
    spin("assert_failure");
    asm volatile("ud2");
}

void panic(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    int i = vsprintf(buf, fmt, ap);
    va_end(ap);

    printk("\n[ERROR]: panic"
                "\n[ERROR]: %s\n", buf);
    spin("panic");
    asm volatile("ud2");
}