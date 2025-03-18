#include <onixs/onixs.h>
#include <onixs/console.h>
#include <onixs/debug.h>
#include <onixs/global.h>
#include <onixs/interrupt.h>
#include <onixs/memorry.h>

void intr_test(){

}

void kernel_init(){
    console_clear();
    memory_map_init();
    mapping_init();
    interrupt_init();

    interrupt_enable();
    bool intr = interrupt_disable_ret();
    interrupt_set_state(intr);
    LOGK("before: %d\n", intr);
    LOGK("now: %d\n", interrupt_get_state());

    hang(true);
}