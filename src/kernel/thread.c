#include <onixs/interrupt.h>
#include <onixs/syscall.h>
#include <onixs/debug.h>
#include <onixs/keyboard.h>
#include <onixs/stdio.h>
#include <onixs/printk.h>
#include <onixs/task.h>
#include <onixs/arena.h>
#include <onixs/device.h>
#include <onixs/fs.h>

void idle_thread()
{
    interrupt_enable();
    uint32 counter = 0;
    while (true)
    {
        // LOGK("[INFO]: idle task %d\n",counter++);
        asm volatile(
            "sti\n"
            "hlt\n"
        );
        yield();
    }
}

void init_thread()
{
    char temp[100];
    dev_init();
    test();
    task_to_user_mode();
}

void test_thread()
{
    uint32 counter = 0;
    interrupt_enable();
    while(true){
        // test();
        sleep(10);
    }
}