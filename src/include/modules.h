#ifndef __MODULES_H__
#define __MODULES_H__

#include "system.h"
#include "vfs.h"
#include "paging.h"
#include "kheap.h"
#include "debug.h"
#include "elf_loader.h"
#include "logdisk.h"

typedef int (*module_init)(int argc, char** argv);
typedef int (*module_fini)();

typedef struct module_info_struct
{
    const char name[32];
    module_init init;
    module_fini fini;
}module_info_t;

typedef struct module_entry_struct
{
    module_info_t* info;
    uint32_t mod_size;
    uint32_t* ptr;

    list_t* symbols;
}module_entry_t;

#define MODULE_ALLOCATION_ADDRESS 0xA0000000
#define MODULE_ALLOCATION_END     0xAFFFFFFF

int load_module(char** argv);

#endif /*__MODULES_H__*/