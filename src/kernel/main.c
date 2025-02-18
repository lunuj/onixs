#include <onixs/onixs.h>
#include <onixs/types.h>
#include <onixs/io.h>
#include <onixs/console.h>
#include <onixs/string.h>
#include <onixs/assert.h>
#include <onixs/stdio.h>
#include <onixs/debug.h>
#include <onixs/global.h>

void kernel_init(){
    console_clear();
    gdt_init();
    while(1){
        printk("Hello world Onixs!\r\n");
    }
}