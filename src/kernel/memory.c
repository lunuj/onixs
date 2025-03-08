#include <onixs/memorry.h>

uint32 total_pages;
uint32 free_pages;
uint32 memory_size;
uint32 memory_base;

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

