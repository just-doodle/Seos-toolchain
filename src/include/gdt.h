#ifndef __GDT_H__
#define __GDT_H__

#include "system.h"
#include "printf.h"
#include "string.h"

extern void flush_gdt(uint32_t gdt_ptr);

typedef struct gdt_entry
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
}__attribute__((packed))gdt_entry_t;

typedef struct gdt_ptr
{
    uint16_t limit;
    uint32_t base;
}__attribute__((packed))gdt_ptr_t;

#define GDT_MAX_ENTRIES 8
#define GDT_GET_USER_SELECTOR(idx) (idx | 0x03)

extern gdt_entry_t entries[GDT_MAX_ENTRIES];

void init_gdt();
void gdt_reinit();
void gdt_setGate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

#endif /*__GDT_H__*/