#include <onixs/onixs.h>
#include <onixs/console.h>
#include <onixs/debug.h>
#include <onixs/interrupt.h>
#include <onixs/memorry.h>
#include <onixs/task.h>
#include <onixs/gate.h>

void kernel_init(){
    console_clear();
    memory_map_init();
    mapping_init();
    interrupt_init();
    task_init();
    syscall_init();

    asm volatile("movl $0, %eax");
    asm volatile("int $0x80");
    hang(true);
}