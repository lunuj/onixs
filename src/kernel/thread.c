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
    int status;
    while(true)
    {
        while(1){
            pid_t pid = fork();
            if (pid){
                printf("parent %d %d %d\n", pid, getpid(), getppid());
                // sleep(1000);
                pid_t child = waitpid(pid, &status);
                printf("wait pid %d status %d %d\n", child, status, time());
            }else{
                printf("child %d %d %d\n", pid, getpid(), getppid());
                sleep(2000);
                exit(0);
            }
            sleep(1000);
        }
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
        // keyboard_read(&ch
        // printk("%c",'t');
        // interrupt_set_state(intr);
        test();
        printf("test thread %d %d %d\n", getpid(), getppid(), counter++);
        sleep(500);
    }
}