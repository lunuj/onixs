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

static void user_init_thread()
{
    // TEST
    char buf[256];

    chroot("/d1");

    chdir("/d2");
    getcwd(buf, sizeof(buf));
    printf("current work directory: %s\n", buf);

    while(true)
    {
        char ch;
        read(stdin, &ch, 1);
        write(stdout, &ch, 1);
        sleep(10);
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
    while(true){
        // test();
        sleep(10);
    }
}