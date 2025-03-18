#include <onixs/onixs.h>
#include <onixs/console.h>
#include <onixs/debug.h>
#include <onixs/interrupt.h>
#include <onixs/memorry.h>
#include <onixs/clock.h>
#include <onixs/task.h>

void intr_test(){

}

void kernel_init(){
    console_clear();
    memory_map_init();
    mapping_init();
    interrupt_init();

    clock_init();
    task_init();

    interrupt_enable();

    hang(true);
}