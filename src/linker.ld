ENTRY(_start)

SECTIONS
{
    . = 0x10000;    /* 将代码加载到 0x10000 地址 */
    .text : {
        *(.multiboot2)
        . = ALIGN(64);
        *(.start)
        *(.text)     /* 将所有的 .text 段放入此位置 */
    }

    .data : {
        *(.data)     /* 将所有的数据段放入此位置 */
    }

    .bss : {
        *(.bss)      /* 未初始化的数据段 */
    }
}
