#include <onixs/interrupt.h>
#include <onixs/syscall.h>
#include <onixs/debug.h>
#include <onixs/keyboard.h>
#include <onixs/stdio.h>

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
    uint32 counter = 0;
    interrupt_enable();

    char ch = 0;
    while(true){
        bool intr = interrupt_disable_ret();
        keyboard_read(&ch, 1);
        printk("%c",ch);
        interrupt_set_state(intr);
        // sleep(500);
    }
}

void test_thread()
{
    uint32 counter = 0;
    interrupt_enable();
    while(true){
        // LOGK("[INFO]: test task %d\n", counter++);
        sleep(1000);
    }
}