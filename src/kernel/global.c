#include <onixs/global.h>
#include <onixs/string.h>
#include <onixs/debug.h>

descriptor_t gdt[GDT_SIZE];
pointer_t gdt_ptr;
tss_t tss;

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

    desc = gdt + USER_CODE_IDX;
    descriptor_init(desc, 0, 0xFFFFF);
    desc->segment = 1;     // 代码段
    desc->granularity = 1; // 4K
    desc->big = 1;         // 32 位
    desc->long_mode = 0;   // 不是 64 位
    desc->present = 1;     // 在内存中
    desc->DPL = 3;         // 用户特权级
    desc->type = 0b1010;   // 代码 / 非依从 / 可读 / 没有被访问过

    desc = gdt + USER_DATA_IDX;
    descriptor_init(desc, 0, 0xFFFFF);
    desc->segment = 1;     // 数据段
    desc->granularity = 1; // 4K
    desc->big = 1;         // 32 位
    desc->long_mode = 0;   // 不是 64 位
    desc->present = 1;     // 在内存中
    desc->DPL = 3;         // 用户特权级
    desc->type = 0b0010;   // 数据 / 向上增长 / 可写 / 没有被访问过

    gdt_ptr.base = (uint32)&gdt;
    gdt_ptr.limit = sizeof(gdt) - 1;
}

void tss_init()
{
    memset(&tss, 0, sizeof(tss));

    tss.ss0 = KERNEL_DATA_SELECTOR;
    tss.iobase = sizeof(tss);

    descriptor_t * desc = gdt + KERNEL_TSS_IDX;
    descriptor_init(desc, (uint32)&tss, sizeof(tss) - 1);
    desc->segment = 0;
    desc->segment = 0;     // 系统段
    desc->granularity = 0; // 字节
    desc->big = 0;         // 固定为 0
    desc->long_mode = 0;   // 固定为 0
    desc->present = 1;     // 在内存中
    desc->DPL = 0;         // 用于任务门或调用门
    desc->type = 0b1001;   // 32 位可用 tss

    asm volatile(
        "ltr %%ax\n"
        ::"a"(KERNEL_TSS_SELECTOR)
    );
}