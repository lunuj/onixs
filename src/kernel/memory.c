#include <onixs/memorry.h>

static uint32 total_pages;
static uint32 free_pages;
static uint32 memory_size;
static uint32 memory_base;

static uint32 start_page = 0;
static uint8 *memory_map;
static uint32 memory_map_pages;

#define used_pages() (total_pages - free_pages)

/**
 * @brief  获取页目录地址
 * @retval [uint32] 页目录地址
 * @note 
 */
uint32 get_cr3(){
    asm volatile("movl %cr3, %eax\n");
}
/**
 * @brief  设置页目录地址
 * @param  pde 存放页目录的地址
 * @retval 无
 * @note pde需位页地址的起始地址，ASSERT_PAGE(pde)
 */
void set_cr3(uint32 pde){
    ASSERT_PAGE(pde);
    asm volatile("movl %%eax, %%cr3\n"::"a"(pde));
}

/**
 * @brief  启用分页机制
 * @retval 无
 * @note 修改cr0寄存器最高位置一
 */
static void enable_page(){
    asm volatile(
        "movl %cr0, %eax\n"
        "orl $0x80000000, %eax\n"
        "movl %eax, %cr0\n"
    );
}

/**
 * @brief  关闭分页机制
 * @retval 无
 * @note 修改cr0寄存器最高位置零
 */
static void disable_page(){
    asm volatile(
        "movl %cr0, %eax\n"
        "andl $0x7FFFFFFF, %eax\n"
        "movl %eax, %cr0\n"
    );
}

/**
 * @brief  初始化页表项
 * @param  entry page_entry_t类型结构体指针
 * @param  index 
 * @retval 无
 * @note 
 */
static void entry_init(page_entry_t * entry, uint32 index){
    *(uint32 *)entry = 0;
    entry->present = 1;
    entry->write = 1;
    entry->user = 1;
    entry->index = index;
}

/**
 * @brief  初始化页目录和页表
 * @retval 无
 * @note 初始化内核页目录，大小4KB，只初始化了第1个页目录，对应1K个页表，
 * 初始化1K个页表，映射4MB内存，对应位置从0x0000_0000到0x003F_FFFF
 */
void mapping_init(){
    //初始化内核页目录，大小4KB，只初始化了第1个页目录，对应1K个页表
    page_entry_t *pde = (page_entry_t *)KERNEL_PAGE_DIR;
    memset(pde, 0, MEMORY_PAGE_SIZE);
    entry_init(&pde[0], IDX(KERNEL_PAGE_ENTRY));

    //初始化1K个页表，映射4MB内存，对应位置从0x0000_0000到0x003F_FFFF
    page_entry_t *pte = (page_entry_t *)KERNEL_PAGE_ENTRY;
    memset(pte, 0, MEMORY_PAGE_SIZE);
    page_entry_t *entry;

    for(size_t tidx = 0; tidx < 1024; tidx ++){
        entry = &pte[tidx];
        entry_init(entry, tidx);
        memory_map[tidx] = 1;
    }

    set_cr3((uint32)pde);
    enable_page();
}

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
    memory_map_pages  = div_round_up(total_pages, MEMORY_PAGE_SIZE);
    LOGK("[INFO]: Memory map page count %d\n", memory_map_pages);
    
    free_pages -= memory_map_pages;
    memset((void *) memory_map, 0, memory_map_pages * MEMORY_PAGE_SIZE);

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