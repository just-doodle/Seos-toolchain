#include "isr.h"

isr_t interrupt_handlers[256];

void register_interrupt_handler(int n, isr_t handler)
{
    if(n < 256)
    {
        printf("[ISR] Registering interrupt handler for int #%d\n", n);
        interrupt_handlers[n] = handler;
    }
}

void isr_handler(registers_t* regs)
{
    if(interrupt_handlers[regs->ino] != 0)
    {
        isr_t handler = interrupt_handlers[regs->ino];
        handler(regs);
    }
    else
    {
        printf("[ISR] No interrupt handler for interrupt %d\n", regs->ino);
    }
    pic_eoi(regs->ino);
}

void enable_interrupts()
{
    ASM_FUNC("sti");
    printf("[ISR] Interrupts enabled\n");
}

void disable_interrupts()
{
    ASM_FUNC("cli");
    printf("[ISR] Interrupts disabled\n");
}