#ifndef __IDT_H__
#define __IDT_H__

#include "system.h"
#include "printf.h"
#include "isr.h"

typedef struct idt_entry
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

typedef struct idt_ptr
{
   uint16_t limit;
   uint32_t base;
} __attribute__((packed)) idt_ptr_t;

extern idt_entry_t idt_entries[256];

void init_idt();
void idt_reinit();
void set_idtGate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);

#endif /*__IDT_H__*/