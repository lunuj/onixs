#include <onixs/interrupt.h>
#include <onixs/syscall.h>
#include <onixs/debug.h>
#include <onixs/mutex.h>

mutex_t mutex_log;

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
    mutex_init(&mutex_log);
    interrupt_enable();
    while(true){
        mutex_lock(&mutex_log);
        LOGK("[INFO]: init task %d\n", counter++);
        mutex_unlock(&mutex_log);
    }
}

void test_thread()
{
    uint32 counter = 0;
    interrupt_enable();
    while(true){
        mutex_lock(&mutex_log);
        LOGK("[INFO]: test task %d\n", counter++);
        mutex_unlock(&mutex_log);
    }
}