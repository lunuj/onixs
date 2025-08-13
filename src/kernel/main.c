#include <onixs/onixs.h>
#include <onixs/console.h>
#include <onixs/debug.h>
#include <onixs/interrupt.h>
#include <onixs/memory.h>
#include <onixs/task.h>
#include <onixs/clock.h>
#include <onixs/gate.h>
#include <onixs/keyboard.h>
#include <onixs/global.h>
#include <onixs/arena.h>
#include <onixs/time.h>
#include <onixs/ide.h>
#include <onixs/buffer.h>

extern void inode_init();
extern void super_init();
extern void file_init();
extern int ramdisk_init();

void kernel_init(){
    console_clear();
//  内存管理
    memory_map_init();
    mapping_init();
    tss_init();
    arena_init();
//  进程调度
    interrupt_init();
    clock_init();
    syscall_init();
    task_init();
//  设备驱动
    time_init();
    ide_init();
    keyboard_init();
//  文件系统
    buffer_init();
    inode_init();
    super_init();
    file_init();
    ramdisk_init();

    interrupt_enable();
    hang(true);
}