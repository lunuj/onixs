#include <onixs/stdio.h>
#include <onixs/debug.h>
#include <onixs/types.h>
#include <onixs/task.h>
#include <onixs/memorry.h>
#include <onixs/interrupt.h>
#include <onixs/syscall.h>

static task_t * task_table[TASK_NUMBER];
static list_t block_list;            // 任务默认阻塞链表
/**
 * @brief  获取当前任务队列中空余地址
 * @retval 任务的地址页
 * @note 
 */
static task_t * task_getFreeTask(){
    for (size_t i = 0; i < TASK_NUMBER; i++)
    {
        if(task_table[i] == NULL){
            task_table[i] = (task_t *)alloc_kpage(1);
            return task_table[i];
        }
    }
    panic("[ERROR]: no more tasks");
}

/**
 * @brief  在任务队列中查找非当前运行任务的处于某种状态的任务
 * @param  state 要查找的任务状态
 * @retval 任务的地址页
 * @note 此函数不可开启中断，返回的任务为最高优先级或最晚运行的任务
 */
static task_t * task_searchTask(task_state_t state){
    assert(!interrupt_get_state());
    task_t * task = NULL;
    task_t * current = running_task();
    for (size_t i = 0; i < TASK_NUMBER; i++)
    {
        task_t * ptr = task_table[i];
        if(ptr == NULL)
            continue;
        if(ptr->state != state)
            continue;
        if(ptr == current)
            continue;
        if(task == NULL || task->ticks < ptr->ticks || (task->ticks == ptr->ticks & ptr->jiffies < task->jiffies))
            task = ptr;
    }
    return task;
}

task_t *running_task(){
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n");
}

void schedule(){
    assert(!interrupt_get_state()); // 不可中断
    task_t *current = running_task();
    task_t *next = task_searchTask(TASK_READY);

    assert(next != NULL);
    assert(next-> magic == ONIXS_MAGIC);

    if(current->state == TASK_RUNNING){
        current->state = TASK_READY;
    }

    next->state = TASK_RUNNING;
    if(next == current)
        return;

    task_switch(next);
}

static task_t * task_create(target_t target, const char * name, uint32 priority, uint32 uid){
    task_t * task = task_getFreeTask();
    memset(task, 0, MEMORY_PAGE_SIZE);
    uint32 stack = (uint32)task + MEMORY_PAGE_SIZE;
    stack -= sizeof(task_frame_t);
    task_frame_t *frame = (task_frame_t *)stack;

    frame->ebx = 0x11111111;
    frame->esi = 0x22222222;
    frame->edi = 0x33333333;
    frame->ebp = 0x44444444;
    frame->eip = (void *)target;

    strcpy((char *)task->name, name);
    task->stack = (uint32 *)stack;
    task->priority = priority;
    task->ticks = priority;
    task->jiffies = 0;
    task->state = TASK_READY;
    task->uid = uid;
    task->vmap = &kernel_map;
    task->pde = KERNEL_PAGE_DIR;
    task->magic = ONIXS_MAGIC;

    return task;
}

static void task_setup(){
    task_t * task = running_task();
    task->magic = ONIXS_MAGIC;
    task->ticks = 1;

    memset(task_table, 0, sizeof(task_table));
}

uint32 thread_a(){
    interrupt_enable();
    while(1){
        printk("A");
        test();
    }
    return 0;
}

uint32 thread_b(){
    interrupt_enable();
    while(1){
        printk("B");
        test();
    }
    return 0;
}

uint32 thread_c(){
    interrupt_enable();
    while(1){
        printk("C");
        test();
    }
    return 0;
}

void task_init(){
    list_init(&block_list);
    task_setup();

    task_create(thread_a, "a", 5, KERNEL_USER);
    task_create(thread_b, "b", 5, KERNEL_USER);
    task_create(thread_c, "c", 5, KERNEL_USER);
}

void task_block(task_t * task, list_t * blist, task_state_t state)
{
    assert(!interrupt_get_state());
    assert(task->node.prev == NULL);
    assert(task->node.next == NULL);
    assert(state != TASK_READY && state != TASK_RUNNING);

    if(blist == NULL){
        blist = &block_list;
    }
    list_push(blist, &task->node);

    task->state = state;
    task_t * current = running_task();
    if(current == task){
        schedule();
    }
}

void task_unblock(task_t * task)
{
    assert(!interrupt_get_state());

    list_remove(&task->node);
    
    assert(task->node.prev == NULL);
    assert(task->node.next == NULL);

    task->state = TASK_READY;
}