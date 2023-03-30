#include "sym.h"

#include "kheap.h"

#include "symbols.h"

multiboot_info_t *m_info;
uint32_t n_syms = 0;

symbol_t* symbols;

void init_symDB()
{
    symbols = get_symDB();
    get_nsyms();
}

void get_nsyms()
{
    while(strcmp("EOS", symbols[n_syms++].name) != 0);
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
    for (int i = 0; i < n_syms; i++)
    {
		sym = &symbols[i];
		uint32_t offset = get_offset(addr, sym);
		if (offset)
			return (symoff_t){sym->name, offset};
	}

	return (symoff_t){"????", 0};

}

void printsym()
{
    symbol_t* sym;
    for(int i = 0; i < n_syms; i++)
    {
        sym = &symbols[i];
        printf("0x%x-0x%x: %s\n", sym->addr, sym->size, sym->name);
    }
}