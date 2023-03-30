#ifndef __SYM_H__
#define __SYM_H__

#include "system.h"
#include "string.h"
#include "printf.h"
#include "elf.h"

typedef struct symoff
{
    char* name;
    uint32_t offset;
}symoff_t;

void init_symDB();
symoff_t get_symbol(uint32_t addr);
void printsym();

#endif /*__SYM_H__*/