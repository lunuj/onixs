ENTRY(_start)

SECTIONS
{
  /* 程序入口基地址 */
  . = 0x01001000;

  /* .text 段：指令，页对齐 */
  .text ALIGN(0x1000) : {
    *(.text)
    *(.text.*)
  }

  /* .rodata 段（可选） */
  .rodata ALIGN(0x1000) : {
    *(.rodata)
    *(.rodata.*)
  }

  /* .data 段：已初始化数据，页对齐 */
  .data ALIGN(0x1000) : {
    *(.data)
    *(.data.*)
  }

  /* .bss 段：未初始化数据，页对齐 */
  .bss ALIGN(0x1000) : {
    *(.bss)
    *(.bss.*)
    *(COMMON)
  }
}
