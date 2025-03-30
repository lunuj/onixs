#include <onixs/interrupt.h>
#include <onixs/syscall.h>
#include <onixs/debug.h>
#include <onixs/mutex.h>

lock_t lcok_log;

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
    lock_init(&lcok_log);
    interrupt_enable();
    while(true){
        lock_acquire(&lcok_log);
        LOGK("[INFO]: init task %d\n", counter++);
        lock_release(&lcok_log);
    }
}

void test_thread()
{
    uint32 counter = 0;
    interrupt_enable();
    while(true){
        lock_acquire(&lcok_log);
        LOGK("[INFO]: test task %d\n", counter++);
        lock_release(&lcok_log);
    }
}