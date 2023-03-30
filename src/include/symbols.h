
#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__

#include "system.h"

typedef struct symbol
{
  const char *name;
  uint32_t addr;
  uint32_t size;
}symbol_t;

symbol_t* get_symDB();

#endif /*__SYMBOLS_H__*/
