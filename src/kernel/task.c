#include <onixs/stdio.h>
#include <onixs/debug.h>
#include <onixs/task.h>

#define PAGE_SIZE 0x1000

task_t *a = (task_t *)0x1000;
task_t *b = (task_t *)0x2000;

extern void task_switch(task_t *next);

task_t *running_task(){
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n");
}

void schedule(){
    task_t *current = running_task();
    task_t *next = current == a ? b : a;
    task_switch(next);
}

uint32 thread_a(){
    while(1){
        printk("A");
        schedule();
    }
}

uint32 thread_b(){
    while(1){
        printk("B");
        schedule();
    }
}

static void task_create(task_t *task, target_t target){
    uint32 stack = (uint32)task + PAGE_SIZE;
    stack -= sizeof(task_frame_t);
    task_frame_t *frame = (task_frame_t *)stack;
    frame->ebx = 0x11111111;
    frame->esi = 0x22222222;
    frame->edi = 0x33333333;
    frame->ebp = 0x44444444;
    frame->eip = (void *)target;

    task->stack = (uint32 *)stack;
}

void task_init(){
    task_create(a, thread_a);
    task_create(b, thread_b);
    schedule();
}