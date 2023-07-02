#include "gdt.h"
#include "bit.h"
#include "debug.h"
#include "isr.h"

gdt_entry_t entries[GDT_MAX_ENTRIES];
gdt_ptr_t ptr;

void gdt_handler(registers_t* registers)
{
    uint32_t ecode = registers->ecode;
    int External = get_bit(ecode, 0);
    int table = get_bit(ecode, 1);
    int index = (ecode >> 2) & 13;

    printf("This fault was happened %s inside %s entry #%d.\n", External == 0 ? "internally" : "externally", (table == 0x00 ? "GDT" : (table == 0x01) ? "IDT" : (table == 0x02) ? "LDT" : "Unknown"), index);
    kernel_panic("GPF");
}

void init_gdt()
{
    printf("[GDT] Configuring GDT pointer...\n");
    ptr.limit = (sizeof(gdt_entry_t) * GDT_MAX_ENTRIES) - 1;
    ptr.base  = (uint32_t)&entries;

    printf("[GDT] Adding entries...\n");

    gdt_setGate(0, 0, 0, 0, 0);
    gdt_setGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_setGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_setGate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_setGate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
    gdt_setGate(0x06, 0, 0xffffffff, 0x9A, 0x0f);
    gdt_setGate(0x07, 0, 0xffffffff, 0x92, 0x0f);

    printf("[GDT] Flushing GDT entries...\n");

    flush_gdt((uint32_t)&ptr);

    register_interrupt_handler(15, gdt_handler);

    printf("[GDT] Initialized successfully\n");
}

void gdt_reinit()
{
    ptr.limit = (sizeof(gdt_entry_t) * GDT_MAX_ENTRIES) - 1;
    ptr.base  = (uint32_t)&entries;

    gdt_setGate(0, 0, 0, 0, 0);
    gdt_setGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_setGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_setGate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_setGate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    flush_gdt((uint32_t)&ptr);
}

void gdt_setGate(int32_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    entries[index].base_low    = (base & 0xFFFF);
    entries[index].base_middle = (base >> 16) & 0xFF;
    entries[index].base_high   = (base >> 24) & 0xFF;
    entries[index].limit_low   = (limit & 0xFFFF);
    entries[index].granularity = (limit >> 16) & 0x0F;
    entries[index].granularity |= gran & 0xF0;
    entries[index].access      = access;
    //printf("[GDT] New entry %d. Base: 0x%06x, Limit: 0x%06x, Access: 0x%02x, Gran: 0x%02x\n", index, base, limit, access, gran);
}