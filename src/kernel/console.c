#include <onixs/console.h>
#include <onixs/io.h>
#include <onixs/string.h>
#include <onixs/interrupt.h>
#include <onixs/device.h>

static uint32 screen;
static uint32 cursor;
static uint32 x, y;

static void get_screen(){
    outb(CRT_ADDR_REG, CRT_START_ADDR_H);
    screen = inb(CRT_DATA_REG) << 8;
    outb(CRT_ADDR_REG, CRT_START_ADDR_L);
    screen |= inb(CRT_DATA_REG);
}

static void set_screen(){
    outb(CRT_ADDR_REG, CRT_START_ADDR_L);
    outb(CRT_DATA_REG,  screen & 0xFF);
    outb(CRT_ADDR_REG, CRT_START_ADDR_H);
    outb(CRT_DATA_REG, (screen >> 8)&0xFF);
}

static void get_cursor(){
    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    cursor = inb(CRT_DATA_REG) << 8;
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    cursor |= inb(CRT_DATA_REG);

    uint32 delta = cursor - screen;
    x = delta % WIDTH;
    y = delta / WIDTH;
}

static void set_cursor(){
    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    outb(CRT_DATA_REG, (cursor >> 8)&0xFF);
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    outb(CRT_DATA_REG, cursor &0xFF);

    uint32 delta = cursor - screen;
    x = delta % WIDTH;
    y = delta / WIDTH;
}

static void scroll_up(uint16 num){
    if(CONSOLE_PAGE_SIZE*2 + screen * 2 + ROW_SIZE < MEM_SIZE){
        memcpy(MEM_BASE_PTR + CONSOLE_PAGE_SIZE*2 + screen * 2, MEM_BASE_PTR + screen*2 + ROW_SIZE, SCR_SIZE - ROW_SIZE);
        screen += CONSOLE_PAGE_SIZE;
        cursor += CONSOLE_PAGE_SIZE;
        console_clearline();
    }else{
        memcpy(MEM_BASE_PTR, MEM_BASE_PTR + screen*2 + ROW_SIZE, SCR_SIZE - ROW_SIZE);
        cursor -= screen;
        screen = 0;
        console_clearline();
    }
    set_screen();
    set_cursor();
}

static void command_bs(){
    if(x){
        uint16 * ptr = MEM_BASE_PTR;
        cursor--;
        *(ptr + cursor) = CRT_BLOCK;
        set_cursor();
    }
}

static void command_del(){
    uint16 * ptr = MEM_BASE_PTR;
    *(ptr + cursor) = CRT_BLOCK;
}

static void command_cr(){
    cursor -= x;
    set_cursor();
}

static void command_lf(){
    if(y + 1 < HEIGHT){
        cursor += WIDTH;
        set_cursor();
        return;
    }
    scroll_up(1);
}

int32 console_write(void *dev, char *buf, uint32 count)
{
    char ch;
    bool irqt = interrupt_disable_ret();
    uint16 * ptr = MEM_BASE_PTR;
    int32 nr = 0;
    while (nr++<count)
    {
        ch = *buf++;
        switch (ch)
        {
        case NUL:
            break;
        case LF:
            command_cr();
            command_lf();
            break;
        case CR:
            command_cr();
            break;
        case BS:
            command_bs();
            break;
        case DEL:
            command_del();
            break;
        case FF:
            command_lf();
            break;
        default:
            *(ptr + cursor) = (CRT_ATTR<<8) | ch;
            if(cursor - screen == 80*25 - 1){
                command_cr();
                command_lf();
            }else{
                cursor++;
                set_cursor();
            }
            break;
        }
    }
    interrupt_set_state(irqt);
    return nr;
}

void console_clearline(){
    uint16 * ptr = MEM_BASE_PTR;
    for (uint8 i = 0; i < WIDTH; i++)
    {
        *(ptr+ i + cursor) = CRT_BLOCK;
    }
}

void console_clear()
{
    screen = 0;
    cursor = 0;
    x = y = 0;
    set_screen();
    set_cursor();
    uint16 * ptr = MEM_BASE_PTR;
    while ((uint32)ptr < MEM_END)
    {
        *ptr++ = CRT_BLOCK & 0x00FF | (CRT_ATTR << 8);
    }
}

void console_init()
{
    console_clear();
    device_install(
        DEV_CHAR, DEV_CONSOLE,
        NULL, "console", 0,
        NULL, NULL, console_write);
}