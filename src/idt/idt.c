#include "idt.h"
#include "config.h"
#include "kernel.h"
#include "memory/memory.h"
#include "io/io.h"

struct idt_descriptor idt_desc[MYOS_TOTAL_INTERRUPTS];
struct idtr_descriptor idtr_desc;

extern void idt_load(struct idtr_descriptor* ptr);
extern void int21h();
extern void no_interrupt();

void no_interrupt_handler()
{
    outb(0x20,0x20);
}

void int21h_handler()
{
    print("Keyboard pressed\n");
    outb(0x20,0x20);
}

void idt_zero()
{
    print("Divide by zero error\n");
}

void idt_set(int i, void* address)
{
    struct idt_descriptor* desc= &idt_desc[i];
    desc->offset_1=(uint32_t)address & 0x0000ffff;
    desc->selector=KERNEL_CODE_SELECTOR;
    desc->zero=0x00;
    desc->type_attr=0xEE;
    desc->offset_2=(uint32_t)address >>16;
}
void idt_init()
{
    memoryset(idt_desc,0,sizeof(idt_desc));
    idtr_desc.limit =sizeof(idt_desc)-1;
    idtr_desc.base=(uint32_t)idt_desc;
    
    for(int i=0;i<MYOS_TOTAL_INTERRUPTS;i++)
    {
        idt_set(i,no_interrupt);
    }
    idt_set(0,idt_zero);
    idt_set(0x20,int21h);
    //load the idt table
    idt_load(&idtr_desc);
}