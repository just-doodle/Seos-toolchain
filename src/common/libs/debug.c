#include "debug.h"
#include "elf_loader.h"
#include "process.h"

void kernel_panic(const char* message)
{
    text_chcolor(VGA_LIGHT_RED, text_getBG());
    printf("kernel panic: - %s\n", message);
    serialprintf("kernel panic: - %s\n", message);
    while(1);
}

void stack_trace(uint32_t maxframes)
{
    stackframe_t* stk;
    asm("movl %%ebp,%0" : "=r"(stk) ::);
    printf("stack trace:\n");
    for(uint32_t frame = 0; stk && frame < maxframes; frame++)
    {
        printf("\t[0x%06x]: 0x%06x\n", ((uint32_t)stk), stk->eip);
        serialprintf("\t[0x%06x]: 0x%06x\n", ((uint32_t)stk), stk->eip);
        stk = stk->ebp;
    }
}

uint32_t get_offset(uint32_t addr, symbol_t* sym)
{
    uint32_t sym_end = sym->addr + sym->size;
    if (addr >= sym_end)
		return 0;

	if (addr < sym->addr)
		return 0;

	return addr - (sym->addr);
}

symoff_t get_symbol(uint32_t addr)
{
    symbol_t* sym;
    if(current_process->isSYMTAB == 1)
    {
        for(int i = 0; i < current_process->n_syms; i++)
        {
            sym = &(current_process->symtab[i]);
            uint32_t offset = get_offset(addr, sym);
            if(offset)
                return (symoff_t){sym->name, offset};
        }
    }

	return (symoff_t){"????", 0};

}

void backtrace()
{
    stackframe_t* stk;
    asm("movl %%ebp,%0" : "=r"(stk) ::);
    symoff_t symbol;
    printf("stack trace:\n");
    for(uint32_t frame = 0; stk && frame < 100; frame++)
    {
        symbol = get_symbol(stk->eip);
        printf("\t[0x%06x]: 0x%06x {%s+", ((uint32_t)stk), stk->eip, symbol.name);
        printf("0x%x}\n", symbol.offset);
        serialprintf("\t[0x%06x]: 0x%06x {%s+", ((uint32_t)stk), stk->eip, symbol.name);
        serialprintf("0x%x}\n", symbol.offset);
        stk = stk->ebp;
    }
}

#define DUMP_COLS 16

void xxd(void* ptr, uint32_t size)
{
    uint32_t i;
    uint32_t j;

    for(i = 0; i < size + ((size % DUMP_COLS) ? (DUMP_COLS - size % DUMP_COLS) : 0); i++) 
    {
        if(i % DUMP_COLS == 0) 
        {
            serialprintf("0x%06x: ", i);
        }

        if(i < size) 
        {
            serialprintf("%02x ", 0xFF & ((char*)ptr)[i]);
        }
        else
        {
            serialprintf("   ");
        }

        if(i % DUMP_COLS == (DUMP_COLS - 1))
        {
            for(j = i - (DUMP_COLS - 1); j <= i; j++)
            {
                if(j >= size)
                {
                    serialprintf(" ");
                }
                else if(isprint(((char*)ptr)[j]))
                {
                    serialprintf("%c", (((char*)ptr)[j]) & 0xFF);
                }
                else
                {
                    serialprintf(".");
                }
            }
            serialprintf("\n");
        }
    }
}

void xxdf(void* ptr, uint32_t size)
{
    uint32_t i;
    uint32_t j;

    for(i = 0; i < size + ((size % DUMP_COLS) ? (DUMP_COLS - size % DUMP_COLS) : 0); i++) 
    {
        if(i % DUMP_COLS == 0) 
        {
            printf("0x%06x: ", i);
        }

        if(i < size) 
        {
            printf("%02x ", 0xFF & ((char*)ptr)[i]);
        }
        else
        {
            printf("   ");
        }

        if(i % DUMP_COLS == (DUMP_COLS - 1))
        {
            for(j = i - (DUMP_COLS - 1); j <= i; j++)
            {
                if(j >= size)
                {
                    printf(" ");
                }
                else if(isprint(((char*)ptr)[j]))
                {
                    printf("%c", (((char*)ptr)[j]) & 0xFF);
                }
                else
                {
                    printf(".");
                }
            }
            printf("\n");
        }
    }
}