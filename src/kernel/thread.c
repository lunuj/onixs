#include <onixs/interrupt.h>
#include <onixs/syscall.h>
#include <onixs/debug.h>

void idle_thread()
{
    interrupt_enable();
    uint32 counter = 0;
    while (true)
    {
        LOGK("[INFO]: idle task %d\n",counter++);
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
    while(true){
        LOGK("[INFO]: init task %d\n", counter++);
        // test();
    }
}