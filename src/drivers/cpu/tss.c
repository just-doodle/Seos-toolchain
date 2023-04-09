#include "tss.h"

static tss_entry_t kernel_tss;

void init_tss(uint32_t idx, uint32_t kss, uint32_t kesp)
{
    serialprintf("[TSS] Adding TSS pointer to GDT #%d...\n", idx);
    uint32_t base = (uint32_t)&kernel_tss;
    uint32_t limit = base + sizeof(tss_entry_t);
    gdt_setGate(idx, base, limit, 0xE9, 0);

    serialprintf("[TSS] Updating kernel tss entries...\n");
    memset(&kernel_tss, 0, sizeof(tss_entry_t));
    kernel_tss.ss0 = kss;
    kernel_tss.esp0 = kesp;

    kernel_tss.cs = 0x0B;
    kernel_tss.ss = 0x13;
    kernel_tss.ds = 0x13;
    kernel_tss.es = 0x13;
    kernel_tss.fs = 0x13;
    kernel_tss.gs = 0x13;
    
    uint32_t cr3;
    asm volatile("mov %%cr3, %0" : "=r" (cr3));
    kernel_tss.cr3 = cr3;

    serialprintf("[TSS] Flushing TSS...\n");

    flush_tss();

    serialprintf("[TSS] Initialization completed...\n");
}

void tss_set_kernel_stack(uint32_t kss, uint32_t kesp)
{
    kernel_tss.ss0 = kss;
    kernel_tss.esp0 = kesp;
}