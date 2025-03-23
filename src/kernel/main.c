#include <onixs/onixs.h>
#include <onixs/console.h>
#include <onixs/debug.h>
#include <onixs/interrupt.h>
#include <onixs/memorry.h>
#include <onixs/task.h>
#include <onixs/clock.h>
#include <onixs/gate.h>
#include <onixs/list.h>

void kernel_init(){
    console_clear();
    memory_map_init();
    mapping_init();
    interrupt_init();
    // clock_init();
    // task_init();
    // syscall_init();
    // interrupt_enable();

    uint32 count = 3;
    list_t holder;
    list_t * list = &holder;
    list_init(list);
    list_node_t *node;
    while (count--)
    {
        node = (list_node_t *)alloc_kpage(1);
        list_push(list, node);
    }

    while(!list_empty(list))
    {
        node = list_pop(list);
        free_kpage((uint32)node, 1);
    }

    count = 3;
    while (count--)
    {
        node = (list_node_t *)alloc_kpage(1);
        list_pushback(list, node);
    }
    LOGK("[INFO]: size = %d", list_size(list));
    while(!list_empty(list))
    {
        node = list_popback(list);
        free_kpage((uint32)node, 1);
    }
    node = (list_node_t *)alloc_kpage(1);
    list_pushback(list, node);
    LOGK("[INFO]: search %#p ->> %d\n", (uint32)node, list_search(list, node));
    LOGK("[INFO]: search %#p ->> %d\n", 0, list_search(list, 0));
    list_remove(node);
    free_kpage((uint32)node, 1);

    hang(true);
}