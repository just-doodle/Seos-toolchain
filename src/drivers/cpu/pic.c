#include "pic.h"
#include "logdisk.h"

void init_pic()
{
    ldprintf("pic", LOG_INFO, "Initializing...");
    ldprintf("pic", LOG_INFO, "Channel 1 command port: 0x%x", PIC1_COMMAND);
    ldprintf("pic", LOG_INFO, "Channel 2 command port: 0x%x", PIC2_COMMAND);
    ldprintf("pic", LOG_INFO, "Channel 1 data port: 0x%x", PIC1_DATA);
    ldprintf("pic", LOG_INFO, "Channel 2 data port: 0x%x", PIC2_DATA);
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);
    ldprintf("pic", LOG_INFO, "Programmable interrupt controller has been configured and started.");
}

void pic_eoi(uint8_t irq)
{
    if(irq >= 0x28)
        outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}