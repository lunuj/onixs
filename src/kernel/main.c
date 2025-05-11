#include <onixs/onixs.h>
#include <onixs/console.h>
#include <onixs/debug.h>
#include <onixs/interrupt.h>
#include <onixs/memorry.h>
#include <onixs/task.h>
#include <onixs/clock.h>
#include <onixs/gate.h>
#include <onixs/keyboard.h>
#include <onixs/global.h>
#include <onixs/arena.h>
#include <onixs/time.h>
#include <onixs/ide.h>
#include <onixs/buffer.h>

extern void super_init();

void kernel_init(){
    console_clear();
    memory_map_init();
    mapping_init();
    tss_init();
    arena_init();
    time_init();
    buffer_init();
    interrupt_init();
    clock_init();
    ide_init();
    super_init();

    task_init();
    syscall_init();
    keyboard_init();

    interrupt_enable();
    hang(true);
}