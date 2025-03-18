#include <onixs/interrupt.h>
#include <onixs/global.h>
#include <onixs/debug.h>
#include <onixs/stdio.h>
#include <onixs/stdlib.h>
#include <onixs/assert.h>
#include <onixs/io.h>

gate_t idt[IDT_SIZE];
pointer_t idt_ptr;

handler_t handler_table[IDT_SIZE];
extern handler_t handler_entry_table[ENTRY_SIZE];

static char *messages[] = {
    "#DE Divide Error\0",
    "#DB RESERVED\0",
    "--  NMI Interrupt\0",
    "#BP Breakpoint\0",
    "#OF Overflow\0",
    "#BR BOUND Range Exceeded\0",
    "#UD Invalid Opcode (Undefined Opcode)\0",
    "#NM Device Not Available (No Math Coprocessor)\0",
    "#DF Double Fault\0",
    "    Coprocessor Segment Overrun (reserved)\0",
    "#TS Invalid TSS\0",
    "#NP Segment Not Present\0",
    "#SS Stack-Segment Fault\0",
    "#GP General Protection\0",
    "#PF Page Fault\0",
    "--  (Intel reserved. Do not use.)\0",
    "#MF x87 FPU Floating-Point Error (Math Fault)\0",
    "#AC Alignment Check\0",
    "#MC Machine Check\0",
    "#XF SIMD Floating-Point Exception\0",
    "#VE Virtualization Exception\0",
    "#CP Control Protection Exception\0",
};

void interrupt_register(uint32 irq, handler_t handler)
{
    assert(irq >= 0 && irq < 16);
    handler_table[IRQ_MASTER_NR + irq] = handler;
}

void interrupt_mask(uint32 irq, bool enable){
    assert(irq >= 0 && irq < 16);
    uint16 port;
    if (irq < 8)
    {
        port = PIC_M_DATA;
    }
    else
    {
        port = PIC_S_DATA;
        irq -= 8;
    }
    if (enable)
    {
        outb(port, inb(port) & ~(1 << irq));
    }
    else
    {
        outb(port, inb(port) | (1 << irq));
    }
}

// 通知中断控制器，中断处理结束
void send_eoi(int vector)
{
    if (vector >= 0x20 && vector < 0x28)
    {
        outb(PIC_M_CTRL, PIC_EOI);
    }
    if (vector >= 0x28 && vector < 0x30)
    {
        outb(PIC_M_CTRL, PIC_EOI);
        outb(PIC_S_CTRL, PIC_EOI);
    }
}

void exception_handler(
    int vector,
    uint32 edi, uint32 esi, uint32 ebp, uint32 esp,
    uint32 ebx, uint32 edx, uint32 ecx, uint32 eax,
    uint32 gs, uint32 fs, uint32 es, uint32 ds,
    uint32 vector0, uint32 error, uint32 eip, uint32 cs, uint32 eflags)
{
    char *message = NULL;
    if (vector < 22)
    {
        message = messages[vector];
    }
    else
    {
        message = messages[15];
    }
    printk("Exception : [0x%02X] %s \n", vector, messages[vector]);
    printk("\nEXCEPTION : %s \n", messages[vector]);
    printk("   VECTOR : 0x%02X\n", vector);
    printk("    ERROR : 0x%08X\n", error);
    printk("   EFLAGS : 0x%08X\n", eflags);
    printk("       CS : 0x%02X\n", cs);
    printk("      EIP : 0x%08X\n", eip);
    printk("      ESP : 0x%08X\n", esp);
    // 阻塞
    hang(true);
}

void default_handler(int vector){
    send_eoi(vector);
    DEBUGK("[INFO]: [IRQ] [%x] default interrupt called\n", vector);
}

void pic_init(){
    outb(PIC_M_CTRL, 0b00010001); // ICW1: 边沿触发, 级联 8259, 需要ICW4.
    outb(PIC_M_DATA, 0x20);       // ICW2: 起始端口号 0x20
    outb(PIC_M_DATA, 0b00000100); // ICW3: IR2接从片.
    outb(PIC_M_DATA, 0b00000001); // ICW4: 8086模式, 正常EOI

    outb(PIC_S_CTRL, 0b00010001); // ICW1: 边沿触发, 级联 8259, 需要ICW4.
    outb(PIC_S_DATA, 0x28);       // ICW2: 起始端口号 0x28
    outb(PIC_S_DATA, 2);          // ICW3: 设置从片连接到主片的 IR2 引脚
    outb(PIC_S_DATA, 0b00000001); // ICW4: 8086模式, 正常EOI

    outb(PIC_M_DATA, 0b11111111); // 关闭所有中断
    outb(PIC_S_DATA, 0b11111111); // 关闭所有中断
}

void idt_init(){
    //初始化异常处理函数
    for(int i = 0; i < ENTRY_SIZE; i++){
        gate_t *gate = &idt[i];
        handler_t handler = handler_entry_table[i];
        gate->offset0 = (uint32)handler&0xFFFF;
        gate->offset1 = ((uint32)handler>>16)&0xFFFF;
        gate->selector = 1<<3;
        gate->reserved = 0;
        gate->type = 0b1110;
        gate->segmeny = 0;
        gate->DPL = 0;
        gate->present = 1;
    }

    for(int size = 0; size < 0x20; size++){
        handler_table[size] = exception_handler;
    }

    for(int size = 0x20; size < ENTRY_SIZE; size++){
        handler_table[size] = default_handler;
    }
    
    idt_ptr.base = (uint32)idt;
    idt_ptr.limit = sizeof(idt) - 1;
    asm volatile("lidt idt_ptr\n");
}

void interrupt_init(){
    pic_init();
    idt_init();
}

/**
 * @brief  置位 IF 位
 * @retval 无
 * @note 
 */
void _inline interrupt_enable(){
    asm volatile("sti\n");
}

bool _inline interrupt_enable_ret(){
    asm volatile(
        "pushfl\n"              // 将当前 eflags 压入栈中
        "sti\n"
        "popl %eax\n"           // 将 eflags 弹出到 eax
        "shrl $9, %eax\n"        // 将 eax 右移 9 位，得到 IF 位
        "andl $1, %eax\n"        // 获取 IF 位
    );
}

/**
 * @brief  清除 IF 位
 * @retval 无
 * @note 
 */
void _inline interrupt_disable(){
    asm volatile("cli\n");
}

bool _inline interrupt_disable_ret(){
    asm volatile(
        "pushfl\n"              // 将当前 eflags 压入栈中
        "cli\n"
        "popl %eax\n"           // 将 eflags 弹出到 eax
        "shrl $9, %eax\n"        // 将 eax 右移 9 位，得到 IF 位
        "andl $1, %eax\n"        // 获取 IF 位
    );
}

/**
 * @brief  获取当前 IF 位状态
 * @retval 当前 IF 位状态
 * @note 
 */
bool interrupt_get_state(){
    asm volatile(
        "pushfl\n"              // 将当前 eflags 压入栈中
        "popl %eax\n"           // 将 eflags 弹出到 eax
        "shrl $9, %eax\n"        // 将 eax 右移 9 位，得到 IF 位
        "andl $1, %eax\n"        // 获取 IF 位
    );
}

/**
 * @brief  设置当前 IF 位
 * @param  state
 * @retval 无
 * @note 
 */
void interrupt_set_state(bool state){
    if (state)
    {
        interrupt_enable();
    }else{
        interrupt_disable();
    }
}