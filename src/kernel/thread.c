#include <onixs/interrupt.h>
#include <onixs/syscall.h>
#include <onixs/debug.h>
#include <onixs/keyboard.h>
#include <onixs/stdio.h>
#include <onixs/printk.h>
#include <onixs/task.h>
#include <onixs/arena.h>

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

static void user_init_thread()
{
    uint32 counter = 0;
    char ch;
    while(true)
    {
        printf("init thread %d %d %d\n", getpid(), getppid(), counter++);
        sleep(1000);
    }
}

void init_thread()
{
    char temp[100];
    task_to_user_mode(user_init_thread);
}

void test_thread()
{
    uint32 counter = 0;
    interrupt_enable();
    char ch = 0;
    while(true){
        // bool intr = interrupt_disable_ret();
        // keyboard_read(&ch, 1);
        // printk("%c",ch);
        // interrupt_set_state(intr);
        printf("test thread %d %d %d\n", getpid(), getppid(), counter++);
        sleep(2500);
    }
}