#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "system.h"
#include "vga_text.h"
#include "printf.h"

typedef struct stackframe
{
    struct stackframe *ebp;
    uint32_t eip;
}stackframe_t;

typedef struct symbol
{
  const char *name;
  uint32_t addr;
  uint32_t size;
}symbol_t;

typedef struct symoff
{
    char* name;
    uint32_t offset;
}symoff_t;

#define ASSERT(b) ((b) ? (void)0 : kernel_panic(#b))


void panic(char* message, char* file);
void kernel_panic(const char* message);
void kernel_panic_noHalt(const char* message);
void stack_trace(uint32_t maxframes);
void xxd(void* ptr, uint32_t size);
int validate(void* ptr);

void load_kernel_symbols(multiboot_info_t* info);
symoff_t get_symbol(uint32_t addr);
symbol_t* get_kernel_symbol_by_name(char* name);

void backtrace();

#endif /*__DEBUG_H__*/