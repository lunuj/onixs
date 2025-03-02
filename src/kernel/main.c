#include <onixs/onixs.h>
#include <onixs/types.h>
#include <onixs/io.h>
#include <onixs/console.h>
#include <onixs/string.h>
#include <onixs/assert.h>
#include <onixs/stdio.h>
#include <onixs/debug.h>
#include <onixs/global.h>
#include <onixs/task.h>
#include <onixs/interrupt.h>
#include <onixs/stdlib.h>
#include <onixs/clock.h>
#include <onixs/time.h>
#include <onixs/rtc.h>

void kernel_init(){
    console_clear();
    gdt_init();
    interrupt_init();
    rtc_init();
    asm volatile("sti\n");
    hang(true);
}