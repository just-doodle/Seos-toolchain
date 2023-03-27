#include "gdt.h"

gdt_entry_t entries[GDT_MAX_ENTRIES];
gdt_ptr_t ptr;

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

    printf("[GDT] Flushing GDT entries...\n");

    flush_gdt((uint32_t)&ptr);

    printf("[GDT] Initialized successfully\n");
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
    printf("[GDT] New entry %d. Base: 0x%06x, Limit: 0x%06x, Access: 0x%02x, Gran: 0x%02x\n", index, base, limit, access, gran);
}