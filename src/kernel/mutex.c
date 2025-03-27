#include <onixs/mutex.h>
#include <onixs/interrupt.h>
#include <onixs/task.h>

void mutex_init(mutex_t * mutex)
{
    mutex->value = false;
    list_init(&mutex->waiters);
}


void mutex_lock(mutex_t * mutex)
{
    bool intr = interrupt_disable_ret();

    task_t * current = running_task();
    while(mutex->value == true)
    {
        task_block(current, &mutex->waiters, TASK_BLOCKED);
    }

    assert(mutex->value == false);

    mutex->value++;
    assert(mutex->value == true);

    interrupt_set_state(intr);
}

void mutex_unlock(mutex_t * mutex)
{
    bool intr = interrupt_disable_ret();

    assert(mutex->value == true);
    mutex->value--;
    assert(mutex->value == false);

    if(!list_empty(&mutex->waiters))
    {
        task_t * task = element_entry(task_t, node, mutex->waiters.tail.prev);
        assert(task->magic == ONIXS_MAGIC);
        task_unblock(task);
        task_yield();
    }

    interrupt_set_state(intr);
}
