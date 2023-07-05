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
  uint32_t type;
}symbol_t;

typedef struct symoff
{
    char* name;
    uint32_t offset;
}symoff_t;

#define STR2(a) #a
#define STR(a) STR2(a)

#define ASSERT(b) ((b) ? (void)0 : kernel_panic(__FILE__":"STR(__LINE__)": assertion \""#b"\" failed"))

#define KDEBUG_ADD_LABEL(id) serialprintf("DEBUG: "__FILE__":"STR(__LINE__)": %s()"": "#id" label executed\n", __FUNCTION__);

void panic(char* message, char* file);
void kernel_panic(const char* message);
void kernel_panic_noHalt(const char* message);
void stack_trace(uint32_t maxframes);
void xxd(void* ptr, uint32_t size);
int validate(void* ptr);

void load_kernel_symbols(struct multiboot_tag_elf_sections* info);
symoff_t get_symbol(uint32_t addr);
symbol_t* get_kernel_symbol_by_name(char* name);

void backtrace();

#endif /*__DEBUG_H__*/