#include <onixs/stdio.h>
#include <onixs/debug.h>
#include <onixs/types.h>
#include <onixs/task.h>
#include <onixs/global.h>
#include <onixs/memorry.h>
#include <onixs/interrupt.h>
#include <onixs/syscall.h>
#include <onixs/arena.h>

extern tss_t tss;

static list_t block_list;            // 任务默认阻塞链表
static list_t sleep_list;
static task_t * task_table[TASK_NUMBER];
static task_t *idle_task;

/**
 * @brief  获取当前任务队列中空余地址
 * @retval 任务的地址页
 * @note 
 */
static task_t * task_getFreeTask(){
    for (size_t i = 0; i < TASK_NUMBER; i++)
    {
        if(task_table[i] == NULL){
            task_t * task = (task_t *)alloc_kpage(1);
            memset(task, 0, MEMORY_PAGE_SIZE);
            task->pid = i;
            task_table[i] = task;
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

    if(task == NULL && state == TASK_READY){
        task = idle_task;
    }
    return task;
}

task_t *running_task(){
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n");
}

void task_activate(task_t * task)
{
    assert(task->magic == ONIXS_MAGIC);
    if(task->pde != get_cr3())
    {
        set_cr3(task->pde);
    }
    if(task->uid != KERNEL_USER)
    {
        tss.esp0 = (uint32)task + MEMORY_PAGE_SIZE;
    }
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
    task_activate(next);
    task_switch(next);
}

static task_t * task_create(target_t target, const char * name, uint32 priority, uint32 uid){
    task_t * task = task_getFreeTask();
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
    task->brk = KERNEL_MEMORY_SIZE;
    task->magic = ONIXS_MAGIC;

    return task;
}

static void task_setup(){
    task_t * task = running_task();
    task->magic = ONIXS_MAGIC;
    task->ticks = 1;

    memset(task_table, 0, sizeof(task_table));
}

void task_to_user_mode(target_t target)
{
    task_t * task = running_task();

    task->vmap = kmalloc(sizeof(bitmap_t));
    void * buf = (void *)alloc_kpage(1);
    bitmap_init(task->vmap, buf, MEMORY_PAGE_SIZE, KERNEL_MEMORY_SIZE / MEMORY_PAGE_SIZE);

    task->pde = (uint32)copy_pde();
    set_cr3(task->pde);

    uint32 addr = (uint32)task + MEMORY_PAGE_SIZE;

    addr -= sizeof(intr_frame_t);
    intr_frame_t * iframe = (intr_frame_t *)(addr);

    iframe->vector = 0x20;
    iframe->edi = 1;
    iframe->esi = 2;
    iframe->ebp = 3;
    iframe->esp_dummy = 4;
    iframe->ebx = 5;
    iframe->edx = 6;
    iframe->ecx = 7;
    iframe->eax = 8;

    iframe->gs = 0;
    iframe->ds = USER_DATA_SELECTOR;
    iframe->es = USER_DATA_SELECTOR;
    iframe->fs = USER_DATA_SELECTOR;
    iframe->ss = USER_DATA_SELECTOR;
    iframe->cs = USER_CODE_SELECTOR;

    iframe->error = ONIXS_MAGIC;

    iframe->eip = (uint32)target;
    iframe->eflags = (0 << 12 | 0b10 | 1 << 9);
    iframe->esp = USER_STACK_TOP;

    asm volatile(
        "movl %0, %%esp\n"
        "jmp interrupt_exit\n" ::"m"(iframe));

}

void task_init(){
    list_init(&block_list);
    list_init(&sleep_list);
    task_setup();
    idle_task = task_create(idle_thread, "idle", 1, KERNEL_USER);
    task_create(init_thread, "init", 5, NORMAL_USER);
    task_create(test_thread, "test", 5, KERNEL_USER);
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

void task_sleep(uint32 ms)
{
    assert(!interrupt_get_state());

    uint32 ticks = ms/jiffy;
    ticks = ticks > 0 ? ticks : 1;

    task_t * current = running_task();
    current->ticks = jiffies + ticks;

    list_insert_sort(&sleep_list, &current->node, element_node_offset(task_t, node, ticks));
    current->state = TASK_SLEEPING;

    schedule();
}

void task_wakeup()
{
    assert(!interrupt_get_state());
    list_t * list = &sleep_list;
    for (list_node_t * ptr =  list->head.next;ptr != &list->tail;)
    {
        task_t * task = element_entry(task_t, node, ptr);
        if(task->ticks > jiffies){
            break;
        }

        ptr = ptr->next;
        task->ticks = 0;
        task_unblock(task);
    }
    
}

void task_yield()
{
    schedule();
}

pid_t sys_getpid()
{
    task_t * task = running_task();
    return task->pid;
}

pid_t sys_getppid()
{
    task_t * task = running_task();
    return task->ppid;
}

static void task_build_stack(task_t *task)
{
    uint32 addr = (uint32)task + MEMORY_PAGE_SIZE;
    addr -= sizeof(intr_frame_t);
    intr_frame_t *iframe = (intr_frame_t *)addr;
    iframe->eax = 0;

    addr -= sizeof(task_frame_t);
    task_frame_t *frame = (task_frame_t *)addr;

    frame->ebp = 0xaa55aa55;
    frame->ebx = 0xaa55aa55;
    frame->edi = 0xaa55aa55;
    frame->esi = 0xaa55aa55;

    frame->eip = interrupt_exit;

    task->stack = (uint32 *)frame;
}

pid_t task_fork()
{
    task_t * task = running_task();
    assert(task->node.next == NULL && task->node.prev == NULL && task->state == TASK_RUNNING);
    task_t * children = task_getFreeTask();
    pid_t pid = children->pid;
    memcpy(children, task, MEMORY_PAGE_SIZE);

    children->pid = pid;
    children->ppid = task->pid;
    children->ticks = children->priority;
    children->state = TASK_READY;

    // 拷贝用户进程虚拟内存位图
    children->vmap = kmalloc(sizeof(bitmap_t));
    memcpy(children->vmap, task->vmap, sizeof(bitmap_t));

    void * buf = (void *)alloc_kpage(1);
    memcpy(buf, task->vmap->bits, MEMORY_PAGE_SIZE);
    children->vmap->bits = buf;

    children->pde = (uint32)copy_pde();

    task_build_stack(children);

    return children->pid;
}

void task_exit(int status)
{
    task_t * task = running_task();
    assert(task->node.next == NULL && task->node.prev == NULL && task->state == TASK_RUNNING);

    task->state = TASK_DIED;
    task->status = status;

    free_pde(task->pde);
    free_kpage((uint32)task->vmap->bits, 1);
    kfree(task->vmap);

    // 将子进程的父进程赋值为自己的父进程
    for (size_t i = 0; i < TASK_NUMBER; i++)
    {
        task_t *child = task_table[i];
        if (!child)
            continue;
        if (child->ppid != task->pid)
            continue;
        child->ppid = task->ppid;
    }
    LOGK("task 0x%p exit....\n", task);

    task_t * parent = task_table[task->ppid];
    if(parent->state == TASK_WAITTING && (parent->waitpid == -1 || parent->waitpid == task->pid))
    {
        task_unblock(parent);
    }
    schedule();
}

pid_t task_waitpid(pid_t pid, int * status)
{
    task_t * task = running_task();
    task_t * child = NULL;
    while(true){
        bool has_child = false;
        for(size_t i = 2; i < TASK_NUMBER; i++)
        {
            task_t * ptr = task_table[i];
            if(!ptr)
                continue;
            if(ptr->ppid!=task->pid)
                continue;
            if(pid != ptr->pid && pid != -1)
                continue;

            if(ptr->state == TASK_DIED)
            {
                child = ptr;
                task_table[i] = NULL;
                goto rollback;
            }

            has_child = true;
        }
        if(has_child)
        {
            task->waitpid = pid;
            task_block(task, NULL, TASK_WAITTING);
            continue;
        }
        break;
    }

    return -1;

rollback:
    * status = child->status;
    uint32 ret = child->pid;
    free_kpage((uint32)child, 1);
    return ret;
}