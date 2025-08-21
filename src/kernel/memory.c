#include <onixs/memory.h>
#include <onixs/multiboot2.h>
#include <onixs/task.h>
#include <onixs/syscall.h>
#include <onixs/bitmap.h>
#include <onixs/fs.h>

#define used_pages() (total_pages - free_pages)
#define PDE_MASK 0xFFC00000

static uint32 total_pages;
static uint32 free_pages;
static uint32 memory_size;
static uint32 memory_base;

static uint32 start_page = 0;
static uint8 *memory_map;
static uint32 memory_map_pages;

static uint32 KERNEL_PAGE_TABLE[] = {
    0x2000,
    0x3000,
    0x4000,
    0x5000
};

bitmap_t kernel_map;

/**
 * @brief  获取造成缺页中断所访问地址
 * @retval [uint32] 地址
 * @note 
 */
uint32 get_cr2(){
    asm volatile("movl %cr2, %eax\n");
}

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
static _inline void enable_page(){
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
static _inline void disable_page(){
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

void memory_init(uint32 maigc, uint32 addr)
{
    uint32 count;
    if(maigc == ONIXS_MAGIC){
        count = *(uint32 *)addr;
        ards_t *ptr = (ards_t *)(addr + 4);

        for(size_t i = 0; i < count; i++, ptr++){
            SYS_LOG(LOG_INFO, "Memory base %#p\n", ptr->base);
            SYS_LOG(LOG_INFO, "Memory size %#p\n", ptr->size);
            SYS_LOG(LOG_INFO, "Memory type %d\n", ptr->type);
            if(ptr->type == ZONE_VALID && ptr->size > memory_size){
                memory_base = (uint32)ptr->base;
                memory_size = (uint32)ptr->size;
            }
        }
    }else if(maigc == MULTIBOOT2_MAGIC){
        uint32 size = *(unsigned int *)addr;
        multi_tag_t * tag = (multi_tag_t *)(addr + 8);

        SYS_LOG(LOG_INFO, "Announced mbi size %#x\n", size);
        while(tag->type != MULTIBOOT_TAG_TYPE_END){
            if(tag->type == MULTIBOOT_TAG_TYPE_MMAP)
                break;
            tag = (multi_tag_t *)((uint32)tag+((tag->size+7) & ~7));
        }
        multi_tag_mmap_t * mtag = (multi_tag_mmap_t *)tag;
        multi_mmap_entry_t * entry = mtag->entries;
        while((uint32)entry < (uint32)tag + tag->size){
            SYS_LOG(LOG_INFO, "Memory base %#p, size %#p, type %d\n", (uint32)entry->addr, (uint32)entry->len, (uint32)entry->type);
            count++;
            if(entry->type == ZONE_VALID && entry->len > memory_size){
                memory_base = (uint32)entry->addr;
                memory_size = (uint32)entry->len;
            }
            entry = (multi_mmap_entry_t *)((uint32)entry + mtag->entry_size);
        }
    }
    else{
        panic("[ERROR]: Memory init magic unknow %#p\n", maigc);
    }
    SYS_LOG(LOG_INFO, "ARDS count %d\n", count);
    SYS_LOG(LOG_INFO, "Memory base %#p\n", (uint32)memory_base);
    SYS_LOG(LOG_INFO, "Memory type %#p\n", (uint32)memory_size);

    assert(memory_base == MEMORY_BASE);
    assert((memory_size & 0xFFF) == 0);

    total_pages = IDX(memory_size) + IDX(MEMORY_BASE);
    free_pages = IDX(memory_size);

    SYS_LOG(LOG_INFO, "Total_pages %d\n", total_pages);
    SYS_LOG(LOG_INFO, "Free_pages %d\n", free_pages);

    if(memory_size < KERNEL_MEMORY_SIZE){
        panic("[ERROR]: System memory is %dM too small, at least %dM needed\n", memory_size/MEMORY_BASE, KERNEL_MEMORY_SIZE/MEMORY_BASE);
    }
}

void memory_map_init(){
    memory_map = (uint8 *)memory_base;
    memory_map_pages  = div_round_up(total_pages, MEMORY_PAGE_SIZE);
    SYS_LOG(LOG_INFO, "Memory map page count %d\n", memory_map_pages);
    
    free_pages -= memory_map_pages;
    memset((void *) memory_map, 0, memory_map_pages * MEMORY_PAGE_SIZE);

    start_page = IDX(MEMORY_BASE) + memory_map_pages;
    for(size_t i = 0; i < start_page; i++){
        memory_map[i] = 1;
    }

    SYS_LOG(LOG_INFO, "Total pages %d free pages %d\n", total_pages, free_pages);

    uint32 legnth = (IDX(KERNEL_MEMORY_SIZE) - IDX(MEMORY_BASE)) / 8;
    bitmap_init(&kernel_map, (uint8 *)KERNEL_MAP_BITS, legnth, IDX(MEMORY_BASE));
    bitmap_scan(&kernel_map, memory_map_pages);
}

/**
 * @brief  初始化页目录和页表
 * @retval 无
 * @note 初始化内核页目录，大小4KB，只初始化了第2个页目录，对应2K个页表，
 * 初始化2K个页表，映射8MB内存，对应位置从0x0000_0000到0x007F_FFFF
 */
void mapping_init(){
    //初始化内核页目录，大小4KB
    page_entry_t *pde = (page_entry_t *)KERNEL_PAGE_DIR;
    memset(pde, 0, MEMORY_PAGE_SIZE);

    idx_t index = 0;
    for(idx_t didx = 0; didx < (sizeof(KERNEL_PAGE_TABLE) / 4); didx++)
    {
        //初始化内核的第didx个页表，大小1024*4byte=4K
        page_entry_t *pte = (page_entry_t *)KERNEL_PAGE_TABLE[didx];
        memset(pte, 0, MEMORY_PAGE_SIZE);

        //配置内核的第didx个页目录项，该页目录项指向1024个页表项的首地址
        page_entry_t *dentry = &pde[didx];
        entry_init(dentry, IDX((uint32)pte));

        //配置内核的页表项，每个页目录项管理1024个页表项
        for (size_t tidx = 0; tidx < 1024; tidx++, index++)
        {
            //配置内核的第tidx个页表项，共1024个，每个指向一个页索引
            if(index == 0)      //第0页不映射，为造成空指针访问异常
                continue;
            //获取页表指向的页
            page_entry_t *tentry = &pte[tidx];
            entry_init(tentry, index);
            memory_map[index] = 1;
        }
        
    }
    page_entry_t * entry = &pde[1023];
    entry_init(entry, IDX(KERNEL_PAGE_DIR));

    set_cr3((uint32)pde);
    enable_page();
}

static uint32 get_page(){
    for(size_t i = start_page; i < total_pages; i++){
        if(!memory_map[i]){
            memory_map[i] = 1;
            assert(free_pages >= 0);
            free_pages--;
            uint32 page = ((uint32)i) << 12;
            SYS_LOG(LOG_INFO, "Get page %#p\n", page);
            return page;
        }
    }
    panic("[ERROR]: Out of memory\n");
}

/**
 * @brief  获取页目录地址
 * @retval 指向页目录的指针
 * @note 0b1111_1111_11|11_1111_1111|0000_0000_0000
 * 将页目录最后一项指向页目录所在的页，
 * 访问页目录最后一项作为页表地址，
 * 再访问页表最后一项作为页地址，
 * 最后根据页内偏移为0得到页目录地址
 */
static page_entry_t* get_pde(){
    return (page_entry_t *)(0xFFFFF000);
}

/**
 * @brief  获取页地址所在的页表地址
 * @param  vaddr 页地址
 * @retval 指向页表地址的指针
 * @note 0b1111_1111_11|00_0000_0000|0000_0000_0000
 * 将页目录最后一项指向页目录所在的页，
 * 访问页目录最后一项作为页表地址，
 * 再访问页表某一项作为页地址，
 * 最后根据页内偏移为0得到页表地址
 */
static page_entry_t* get_pte(uint32 vaddr, bool create){
    page_entry_t * pde = get_pde();
    uint32 idx = DIDX(vaddr);
    page_entry_t * entry = &pde[idx];
    
    assert(create || (!create && entry->present));

    page_entry_t * table = (page_entry_t *)(PDE_MASK | (idx << 12));
    if(!entry->present){
        SYS_LOG(LOG_INFO, "get and create page table entry for %#p\n", vaddr);
        uint32 page = get_page();
        entry_init(entry, IDX(page));
        memset(table, 0, MEMORY_PAGE_SIZE);
    }
    return table;
}

page_entry_t *get_entry(uint32 vaddr, bool create)
{
    page_entry_t *pte = get_pte(vaddr, create);
    return &pte[TIDX(vaddr)];
}

void flush_tlb(uint32 vaddr){
    asm volatile(
        "invlpg (%0)"::"r"(vaddr):"memory"
    );
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
    SYS_LOG(LOG_INFO, "Put page %#p\n", addr);
}

static uint32 scan_page(bitmap_t * map, uint32 count){
    assert(count > 0);
    int32 index = bitmap_scan(map, count);
    if(index == EOF){
        panic("[ERRIR]: scan page error");
    }
    uint32 addr = PAGE(index);
    SYS_LOG(LOG_INFO, "Scan page %#p count %d\n", addr, count);
    return addr;
}

static void reset_page(bitmap_t * map, uint32 addr, uint32 count){
    ASSERT_PAGE(addr);
    assert(count > 0);
    uint32 index = IDX(addr);

    for (size_t i = 0; i < count; i++)
    {
        assert(bitmap_test(map, index + i));
        bitmap_set(map, index + i, 0);
    }
}

uint32 alloc_kpage(uint32 count){
    assert(count > 0);
    uint32 vaddr = scan_page(&kernel_map, count);
    SYS_LOG(LOG_INFO, "alloc kernel pages %#p count %d\n", vaddr, count);
    return vaddr;
}

void free_kpage(uint32 vaddr, uint32 count){
    ASSERT_PAGE(vaddr);
    assert(count > 0);
    reset_page(&kernel_map, vaddr, count);
    SYS_LOG(LOG_INFO, "free kernel pages %#p count %d\n", vaddr, count);
}

void link_page(uint32 vaddr)
{
    ASSERT_PAGE(vaddr);

    page_entry_t * entry = get_entry(vaddr, true);

    uint32 index = IDX(vaddr);

    if(entry->present)
    {
        return;
    }

    uint32 paddr = get_page();
    entry_init(entry, IDX(paddr));
    flush_tlb(vaddr);

    SYS_LOG(LOG_INFO, "LINK from %#p to %#p\n", vaddr, paddr);
}

void unlink_page(uint32 vaddr)
{
    ASSERT_PAGE(vaddr);

    page_entry_t * pde = get_pde();
    page_entry_t * entry = &pde[DIDX(vaddr)];
    if(!entry->present)
        return;

    entry = get_entry(vaddr, false);    
    if(!entry->present)
        return;

    entry->present = false;

    uint32 paddr = PAGE(entry->index);

    DEBUGK("[INFO]: unlink from %#p to %#p\n");
    put_page(paddr);
    flush_tlb(vaddr);
}

static uint32 copy_page(void * page)
{
    uint32 paddr = get_page();
    uint32 vaddr = 0;

    page_entry_t * entry = get_pte(vaddr, false);
    entry_init(entry, IDX(paddr));
    flush_tlb(vaddr);

    memcpy((void *)0, (void *)page, MEMORY_PAGE_SIZE);
    entry->present = false;
    flush_tlb(vaddr);

    return paddr;
}

page_entry_t * copy_pde()
{
    task_t * task = running_task();
    page_entry_t * pde = (page_entry_t *)alloc_kpage(1);
    memcpy(pde, (void *)task->pde, MEMORY_PAGE_SIZE);

    page_entry_t * entry = &pde[1023];
    entry_init(entry, IDX(pde));

    page_entry_t * dentry;
    for(size_t didx = (sizeof(KERNEL_PAGE_TABLE) / 4); didx < 1023; didx++)
    {
        dentry = & pde[didx];
        if(!dentry->present)
            continue;
        page_entry_t * pte = (page_entry_t *)(PDE_MASK | (didx << 12));
        for(size_t tidx = 0; tidx < 1024; tidx++)
        {
            entry = &pte[tidx];
            if(!entry->present)
                continue;
            assert(memory_map[entry->index] > 0);
            if(!entry->shared)
            {
                entry->write = false;
            }
            memory_map[entry->index]++;
            assert(memory_map[entry->index] < 255);
        }

        uint32 paddr = copy_page(pte);
        dentry->index = IDX(paddr);
    }

    set_cr3(task->pde);
    return pde;
}

void free_pde()
{
    task_t *task = running_task();
    assert(task->uid != KERNEL_USER);

    page_entry_t *pde = get_pde();

    for (size_t didx = (sizeof(KERNEL_PAGE_TABLE) / 4); didx < 1023; didx++)
    {
        page_entry_t *dentry = &pde[didx];
        if (!dentry->present)
        {
            continue;
        }

        page_entry_t *pte = (page_entry_t *)(PDE_MASK | (didx << 12));

        for (size_t tidx = 0; tidx < 1024; tidx++)
        {
            page_entry_t *entry = &pte[tidx];
            if (!entry->present)
            {
                continue;
            }

            assert(memory_map[entry->index] > 0);
            put_page(PAGE(entry->index));
        }

        // 释放页表
        put_page(PAGE(dentry->index));
    }

    // 释放页目录
    free_kpage(task->pde, 1);
    LOGK("free pages %d\n", free_pages);
}

typedef struct page_error_code_t
{
    uint8 present : 1;
    uint8 write : 1;
    uint8 user : 1;
    uint8 reserved0 : 1;
    uint8 fetch : 1;
    uint8 protection : 1;
    uint8 shadow : 1;
    uint16 reserved1 : 8;
    uint8 sgx : 1;
    uint16 reserved2;
} _packed page_error_code_t;

void page_fault(
    int vector,
    uint32 edi, uint32 esi, uint32 ebp, uint32 esp,
    uint32 ebx, uint32 edx, uint32 ecx, uint32 eax,
    uint32 gs, uint32 fs, uint32 es, uint32 ds,
    uint32 vector0, uint32 error, uint32 eip, uint32 cs, uint32 eflags)
{
    assert(vector = 0xe);
    uint32 vaddr = get_cr2();
    LOGK("fault address %#p\n",vaddr);

    page_error_code_t * code = (page_error_code_t *)&error;
    task_t * task = running_task();

    assert(KERNEL_MEMORY_SIZE <= vaddr < USER_STACK_TOP);

    if(code->present)
    {
        assert(code->write);

        page_entry_t * entry = get_entry(vaddr, false);

        assert(entry->present);   // 目前写内存应该是存在的
        assert(!entry->shared);   // 共享内存页，不应该引发缺页
        assert(!entry->readonly); // 只读内存页，不应该被写

        assert(memory_map[entry->index] > 0);
        if(memory_map[entry->index] == 1)
        {
            entry->write = true;
            LOGK("write page for %#p\n", vaddr);
        }else{
            void * page = (void *)PAGE(IDX(vaddr));
            uint32 paddr = copy_page(page);
            memory_map[entry->index]--;
            entry_init(entry, IDX(paddr));
            flush_tlb(vaddr);
            LOGK("copy page for %3p\n", vaddr);
        }
        return;
    }

    if(!code->present && (vaddr >= USER_STACK_BOTTOM || vaddr < task->brk))
    {
        uint32 page = PAGE(IDX(vaddr));
        link_page(page);
        return;
    }
    panic("page fault");
}


int32 sys_brk(void * addr)
{
    SYS_LOG(LOG_INFO, "task brk %#p\n", addr);

    uint32 brk = (uint32)addr;
    ASSERT_PAGE(brk);

    task_t * task = running_task();
    assert(task->uid != KERNEL_USER);

    assert(task->end <= brk && brk <= USER_MMAP_ADDR);

    uint32 old_brk = task->brk;
    if(old_brk > brk)
    {
        for(uint32 page = brk; brk < old_brk; page += MEMORY_PAGE_SIZE)
        {
            unlink_page(page);
        }
    }
    else if(IDX(brk - old_brk) > free_pages)
    {
        return -1;
    }

    task->brk = brk;
    return 0;
}


void *sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    ASSERT_PAGE((uint32)addr);

    uint32 count = div_round_up(length, MEMORY_PAGE_SIZE);
    uint32 vaddr = (uint32)addr;

    task_t *task = running_task();
    if(!vaddr)
    {
        vaddr = scan_page(task->vmap, count);
    }

    assert(vaddr >= USER_MMAP_ADDR && vaddr < USER_STACK_BOTTOM);

    for (size_t i = 0; i < count; i++)
    {
        uint32 page = vaddr + MEMORY_PAGE_SIZE *i;
        link_page(page);
        bitmap_set(task->vmap, IDX(page), true);

        page_entry_t *entry = get_entry(page, false);
        entry->user = true;
        entry->write = false;
        entry->readonly = true;
        if(prot & PROT_WRITE)
        {
            entry->readonly = false;
            entry->write = true;
        }
        if(flags & MAP_SHARED)
        {
            entry->shared = true;
        }
        if(flags & MAP_PRIBATE)
        {
            entry->privat = true;
        }
    }

    if(fd != EOF)
    {
        lseek(fd, offset, SEEK_SET);
        read(fd, (char *)vaddr, length);
    }
    return (void *)vaddr;
}

int sys_munmap(void *addr, size_t length)
{
    task_t *task = running_task();
    uint32 vaddr = (uint32)addr;
    assert(vaddr >= USER_MMAP_ADDR && vaddr < USER_STACK_BOTTOM);

    assert(PAGE(vaddr));

    uint32 count = div_round_up(length, MEMORY_PAGE_SIZE);

    for (size_t i = 0; i < count; i++)
    {
        uint32 page = vaddr + MEMORY_PAGE_SIZE * i;
        unlink_page(page);
        assert(bitmap_test(task->vmap, IDX(page)));
        bitmap_set(task->vmap, IDX(page), false);
    }
    return 0;
}