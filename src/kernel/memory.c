#include <onixs/memorry.h>

static uint32 total_pages;
static uint32 free_pages;
static uint32 memory_size;
static uint32 memory_base;

static uint32 start_page = 0;
static uint8 *memory_map;
static uint32 memory_map_pages;

#define used_pages() (total_pages - free_pages)

void memory_init(uint32 maigc, uint32 addr)
{
    uint32 count;
    ards_t *ptr;
    if(maigc == ONIXS_MAGIC){
        count = *(uint32 *)addr;
        ptr = (ards_t *)(addr + 4);

        for(size_t i = 0; i < count; i++, ptr++){
            LOGK("[INFO]: Memory base %#p\n", ptr->base);
            LOGK("[INFO]: Memory size %#p\n", ptr->size);
            LOGK("[INFO]: Memory type %d\n", ptr->type);
            if(ptr->type == ZONE_VALID && ptr->size > memory_size){
                memory_base = (uint32)ptr->base;
                memory_size = (uint32)ptr->size;
            }
        }
    }else{
        panic("[ERROR]: Memory init magic unknow %#p\n", maigc);
    }
    LOGK("[INFO]: ARDS count %d\n", count);
    LOGK("[INFO]: Memory base %#p\n", (uint32)memory_base);
    LOGK("[INFO]: Memory type %#p\n", (uint32)memory_size);

    assert(memory_base == MEMORY_BASE);
    assert((memory_size & 0xFFF) == 0);

    total_pages = IDX(memory_size) + IDX(MEMORY_BASE);
    free_pages = IDX(memory_size);

    LOGK("[INFO]: Total_pages %d\n", total_pages);
    LOGK("[INFO]: Free_pages %d\n", free_pages);
}

void memory_map_init(){
    memory_map = (uint8 *)memory_base;
    memory_map_pages  = div_round_up(total_pages, PAGE_SIZE);
    LOGK("[INFO]: Memory map page count %d\n", memory_map_pages);
    
    free_pages -= memory_map_pages;
    memset((void *) memory_map, 0, memory_map_pages * PAGE_SIZE);

    start_page = IDX(MEMORY_BASE) + memory_map_pages;
    for(size_t i = 0; i < start_page; i++){
        memory_map[i] = 1;
    }

    LOGK("[INFO]: Total pages %d free pages %d\n", total_pages, free_pages);
}

static uint32 get_page(){
    for(size_t i = start_page; i < total_pages; i++){
        if(!memory_map[i]){
            memory_map[i] = i;
            free_pages--;
            assert(free_pages >= 0);
            uint32 page = ((uint32)i) << 12;
            LOGK("[INFO]: Get page %#p\n", page);
            return page;
        }
    }
    panic("[ERROR]: Out of memory\n");
}

static void put_page(uint32 addr){
    ASSERT_PAGE(addr);
    uint32 idx = IDX(addr);
    assert(idx >= start_page && idx < total_pages);
    assert(memory_map[idx] >= 1);
    memory_map[idx]--;
    if(!memory_map[idx]){
        free_pages++;
    }
    assert(free_pages > 0 && free_pages < total_pages);
    LOGK("[INFO]: Put page %#p\n", addr);
}

void memory_test(){
    uint32 page[10];
    for(size_t i = 0; i < 10; i++){
        page[i] = get_page();
    }

    for(size_t i = 0; i < 10; i++){
        put_page(page[i]);
    }
}