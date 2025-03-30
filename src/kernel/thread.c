#include <onixs/interrupt.h>
#include <onixs/syscall.h>
#include <onixs/debug.h>
#include <onixs/mutex.h>

spinlock_t spinlcok_log;

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
    spin_init(&spinlcok_log);
    interrupt_enable();
    while(true){
        spin_lock(&spinlcok_log);
        LOGK("[INFO]: init task %d\n", counter++);
        spin_unlock(&spinlcok_log);
    }
}

void test_thread()
{
    uint32 counter = 0;
    interrupt_enable();
    while(true){
        spin_lock(&spinlcok_log);
        LOGK("[INFO]: test task %d\n", counter++);
        spin_unlock(&spinlcok_log);
    }
}