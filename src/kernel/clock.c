#include <onixs/io.h>
#include <onixs/interrupt.h>
#include <onixs/assert.h>
#include <onixs/debug.h>
#include <onixs/clock.h>
#include <onixs/task.h>

// 时间片计数器
uint32 volatile jiffies = 0;
uint32 jiffy = JIFFY;

void beep_start(){
    outb(SPEAKER_REG, 0b11);
}

void beep_stop(){
    outb(SPEAKER_REG, inb(SPEAKER_REG) & 0xfc);
}

void clock_handler(int vector)
{
    assert(vector == 0x20);
    send_eoi(vector);
    jiffies++;
    task_t * task = running_task();
    assert(task->magic == ONIXS_MAGIC);

    task->jiffies = jiffies;
    task->ticks--;
    if(!task->ticks){
        task->ticks = task->priority;
        schedule();
    }
}

void pit_init()
{
    outb(PIT_CTRL_REG, 0b00110100);
    outb(PIT_CHAN0_REG, CLOCK_COUNTER & 0xff);
    outb(PIT_CHAN0_REG, (CLOCK_COUNTER >> 8) & 0xff);

    outb(PIT_CTRL_REG, 0b10110110);
    outb(PIT_CHAN0_REG, BEEP_COUNT & 0xff);
    outb(PIT_CHAN0_REG, (BEEP_COUNT >> 8) & 0xff);
}

void clock_init()
{
    pit_init();
    interrupt_register(IRQ_CLOCK, clock_handler);
    interrupt_mask(IRQ_CLOCK, true);
}
