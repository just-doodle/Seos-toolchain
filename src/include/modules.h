#ifndef __MODULES_H__
#define __MODULES_H__

#include "system.h"
#include "vfs.h"
#include "paging.h"
#include "kheap.h"
#include "debug.h"
#include "elf_loader.h"
#include "logdisk.h"

#define MODULE_METADATA_MAGIC 0x4D4F44FF


typedef int (*module_init)(int argc, char** argv);
typedef int (*module_fini)();

typedef struct module_info_struct
{
    uint32_t magic;
    const char name[32];
    const char init_name[32];
    const char fini_name[32];
}module_info_t;

typedef struct module_entry_struct
{
    module_info_t* info;
    uint32_t mod_size;
    uint32_t* ptr;

    list_t* symbols;
    list_t* dependencies;

    int depended_by;
}module_entry_t;

#define MODULE_DEF(n,init,fini) \
        module_info_t metadata = { \
            .magic = (MODULE_METADATA_MAGIC), \
            .name       = #n, \
            .init_name = #init, \
            .fini_name = #fini, \
        }

#define MODULE_NAME_CONCAT(MODULE_NAME) __mod_dep_list_##MODULE_NAME
#define MODULE_DEPENDENCY_CONCAT(MODULE_NAME, DEP) __mod_dep_list_##MODULE_NAME##_##DEP
#define MODULE_VAR_ATTR_CONCAT(MODULE_NAME, DEP) MODULE_DEPENDENCY_CONCAT(MODULE_NAME, DEP)[32] __attribute__((section("moddeps"), used))
#define MODULE_DEPENDS(MODULE_NAME, n) static const char MODULE_VAR_ATTR_CONCAT(MODULE_NAME, n) = #n

#define MODULE_ALLOCATION_ADDRESS 0xA0000000
#define MODULE_ALLOCATION_END     0xAFFFFFFF

int load_module(char** argv);
int unload_module(char* name);
void print_list_modules();
int module_stopall();

symoff_t get_symoff_from_modules(uint32_t addr);

#endif /*__MODULES_H__*/