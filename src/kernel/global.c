#include <onixs/global.h>
#include <onixs/string.h>
#include <onixs/debug.h>

descriptor_t gdt[GDT_SIZE];
pointer_t gdt_ptr;

void descriptor_init(descriptor_t * desc, uint32 base, uint32 limit)
{
    desc->base_low = base & 0xFFFFFF;
    desc->base_high = (base >> 24)&0xFF;
    desc->limit_low = limit & 0xFFFF;
    desc->limit_high = (limit >> 16) & 0xF;
}

void gdt_init(){
    DEBUGK("gdt init\n");
    
    memset(gdt, 0, sizeof(gdt));

    descriptor_t * desc;
    desc = gdt + KERNEL_CODE_IDX;
    descriptor_init(desc, 0, 0xFFFFF);
    desc->segment = 1;
    desc->granularity = 1;
    desc->big = 1;
    desc->long_mode = 0;
    desc->present = 1;
    desc->DPL = 0;
    desc->type = 0b1010;

    desc = gdt + KERNEL_DATA_IDX;
    descriptor_init(desc, 0, 0xFFFFF);
    desc->segment = 1;
    desc->granularity = 1;
    desc->big = 1;
    desc->long_mode = 0;
    desc->present = 1;
    desc->DPL = 0;
    desc->type = 0b0010;

    gdt_ptr.base = (uint32)&gdt;
    gdt_ptr.limit = sizeof(gdt) - 1;
}