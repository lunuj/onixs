#include <onixs/console.h>
#include <onixs/io.h>
#include <onixs/string.h>

static uint32 screen;
static uint32 cursor;
static uint32 x, y;
static uint8 attr = 7;
static uint16 erase = 0x0720;

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
    if(cursor*2 + ROW_SIZE < MEM_SIZE){
        screen += WIDTH;
        cursor += WIDTH;
        uint16 * ptr = MEM_BASE_PTR;
        for (uint8 i = 0; i < WIDTH; i++)
        {
            *(ptr+ i + cursor) = 0x3520;
        }
    }else{
        memcpy(MEM_BASE_PTR, MEM_BASE_PTR + screen*2, SCR_SIZE);
        cursor -= screen;
        screen = 0;
    }
    set_screen();
    set_cursor();
}

static void command_bs(){
    if(x){
        uint16 * ptr = MEM_BASE_PTR;
        cursor--;
        *(ptr + cursor) = 0x0720;
        set_cursor();
    }
}

static void command_del(){
    uint16 * ptr = MEM_BASE_PTR;
    *(ptr + cursor) = 0x3020;
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

void console_init()
{
    console_clear();
}
void console_write(char *buf, uint32 count)
{
    char ch;
    uint16 * ptr = MEM_BASE_PTR;
    while (count--)
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
            *(ptr + cursor) = (attr<<8) | ch;
            cursor++;
            set_cursor();
            break;
        }
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
        *ptr++ = 0x0720;
    }
}