ENTRY(_start)

SECTIONS
{
    . = 0x10000;    /* 将代码加载到 0x10000 地址 */
    .start : {
        *(.start)
    }

    .text : {
        *(.text)     /* 将所有的 .text 段放入此位置 */
    }

    .data : {
        *(.data)     /* 将所有的数据段放入此位置 */
    }
}
