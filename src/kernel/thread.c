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
void test_recursion()
{
    char tmp[400];
    test_recursion();
}

static void user_init_thread()
{
    uint32 counter = 0;
    char ch;
    while(true)
    {
        // asm volatile("in $0x9,%ax\n");
        test_recursion();
        sleep(100);
        // printf("task is in user mode %d\n", counter++);
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
        void * ptr = kmalloc(0x2000);
        LOGK("kalloc %#p\n", ptr);
        kfree(ptr);
        ptr = kmalloc(1024);
        LOGK("kalloc %#p\n", ptr);
        kfree(ptr);
        ptr = kmalloc(54);
        LOGK("kalloc %#p\n", ptr);
        kfree(ptr);
    
        // bool intr = interrupt_disable_ret();
        // keyboard_read(&ch, 1);
        // printk("%c",ch);
        // interrupt_set_state(intr);
        sleep(2500);
    }
}