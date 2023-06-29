#include "modules.h"
#include "kernelfs.h"

uint32_t module_mapper_addr = MODULE_ALLOCATION_ADDRESS;
uint32_t module_mapper_end = MODULE_ALLOCATION_END;

list_t* modules = NULL;

#define MODULE_LOADER_NO_EXTRA_INFO 1

#if MODULE_LOADER_NO_EXTRA_INFO
#define __K_MLDP__(fmt, ...)
#else
#define __K_MLDP__(fmt, ...) ldprintf("Module loader", LOG_DEBUG, fmt, __VA_ARGS__)
#endif

void* module_alloc(size_t sz, int writable)
{
    void* ret = module_mapper_addr;
    module_mapper_addr += sz;
    if(validate(ret) != 1)
    {
        alloc_region(kernel_page_dir, ret, ret+sz, 0, 1, writable);
    }

    return ret;
}

int module_exists(char* name)
{
    foreach(t, modules)
    {
        module_entry_t* m = t->val;
        if(strcmp(m->info->name, name) == 0)
            return 1;
    }

    return 0;
}

// void update_module_kfs_entry()
// {
//     if(validate(modules) != 1)
//         return;

//     char* l = zalloc((list_size(modules) * 512));

//     foreach(k, modules)
//     {
//         module_entry_t* e = k->val;
//         if(validate(e))
//             sprintf(l+strlen(l), "%06x %d %s\n", e->ptr, e->depended_by, e->info->name);
//     }
//     kernelfs_addcharf("/proc", "modules", "%s", l);
//     free(l);
// }

elf_sym_t* get_module_symbol_by_name(char* name, elf_sym_t* stable, char* strtab, int nsyms)
{
    for(uint32_t i = 0; i < nsyms; i++)
    {
        elf_sym_t* sym = &stable[i];
        char* symname = strdup(&(strtab[stable[i].st_name]));
        if(strcmp(name, symname) == 0)
            return sym;
        else
        {
            free(symname);
        }
    }
}

symbol_t* get_symbol_from_modulelist(char* name, module_entry_t* entry)
{
    if(validate(entry) != 1)
        return NULL;
    foreach(symbols_node, entry->symbols)
    {
        symbol_t* s = symbols_node->val;
        if(strcmp(name, s->name) == 0)
        {
            return s;
        }
    }
    return NULL;
}

symbol_t* get_symbol_from_modules(char* name)
{
    foreach(module_node, modules)
    {
        module_entry_t* entry = module_node->val;
        foreach(symbols_node, entry->symbols)
        {
            symbol_t* s = symbols_node->val;
            if(strcmp(name, s->name) == 0)
            {
                return s;
            }
        }
    }
    return NULL;
}

list_t* get_symbol_list_from_last_module()
{
    if(validate(modules) != 1)
        return NULL;

    list_t* symbols = NULL;
    module_entry_t* mentry = list_peek_back(modules);
    if(validate(mentry) != 1)
        return NULL;
    symbols = mentry->symbols;
    if(validate(symbols) != 1)
        return NULL;
    return symbols;
}

void add_sym_to_modentry(char* name, uint32_t addr, uint32_t size, uint32_t type, module_entry_t* me)
{
    symbol_t* s = ZALLOC_TYPES(symbol_t);
    s->name = name;
    s->addr = addr;
    s->size = size;
    s->type = type;

    if(me->symbols == NULL)
        me->symbols = list_create();

    list_push(me->symbols, s);
}

static const char* get_stt(elf_sym_t* sym)
{
    switch (sym->st_info)
    {
    case STT_FILE:
    {
        return "FILE   ";
    }break;
    case STT_FUNC:
    {
        return "FUNC   ";
    }break;
    case STT_SECTION:
    {
        return "SECTION";
    }break;
    case STT_OBJECT:
    {
        return "OBJECT ";
    }break;
    case STT_NOTYPE:
    {
        return "NOTYPE ";
    }break;
    default:
    {
        return "UNKNOWN";
    }break;
    };
}

int module_isLoaded(char* name)
{
    foreach(l, modules)
    {
        module_entry_t *e = l->val;
        if(strcmp(e->info->name, name) == 0)
        {
            return 1;
        }
    }
    return 0;
}

module_entry_t* get_module_entry_by_name(char* name)
{
    foreach(l, modules)
    {
        module_entry_t *e = l->val;
        if(strcmp(e->info->name, name) == 0)
        {
            __K_MLDP__("ENTRY_NAME: %s", name);
            return e;
        }
    }
    return NULL;
}

static const char* get_scope(elf_sym_t* sym)
{
    switch(ELF32_ST_BIND(sym->st_info))
    {
    case STB_LOCAL:
    {
        return "LOCAL  ";
    }break;
    case STB_GLOBAL:
    {
        return "GLOBAL ";
    }break;
    case STB_WEAK:
    {
        return "WEAK   ";
    }break;
    case STB_LOOS:
    {
        return "LOOS   ";
    }break;
    case STB_HIOS:
    {
        return "HIOS   ";
    }break;
    case STB_LOPROC:
    {
        return "LOPROC ";
    }break;
    case STB_HIPROC:
    {
        return "HIPROC ";
    }break;
    default:
    {
        return "UNKNOWN";
    }break;
    };
}

int check_module(uint8_t* buf, uint32_t size)
{
    if(validate(buf) != 1)  
    {
        ldprintf("Module loader", LOG_ERR, "Given module does not exist");
        return 10;
    }
    elf_header_t* head = buf;

    int code = 0;
    if(head->e_ident[EI_MAG0] != ELFMAG0)
        code = 1;
    if(head->e_ident[EI_MAG1] != ELFMAG1)
        code = 2;
    if(head->e_ident[EI_MAG2] != ELFMAG2)
        code = 3;
    if(head->e_ident[EI_MAG3] != ELFMAG3)
        code = 4;

    if(head->e_ident[EI_CLASS] != ELFCLASS32)
        code = 5;
    if(head->e_ident[EI_DATA] != ELFDATA2LSB)
        code = 6;
    if(head->e_ident[EI_VERSION] != EV_CURRENT)
        code = 7;
    if(head->e_machine != EM_386)
        code = 8;
    if(head->e_type != ET_REL)
        code = 9;

    if(code == 0)
        return 1;

    if(code <= 4)
    {
        ldprintf("Module loader", LOG_ERR, "Given module does not have an valid elf signature.");
        return 0;
    }
    else if(code == 5)
    {
        ldprintf("Module loader", LOG_ERR, "Given module is not compiled for %s.", KERNEL_ARCH);
        return 0;
    }
    else if(code == 6)
    {
        ldprintf("Module loader", LOG_ERR, "Given module uses a invalid data orientation.");
        return 0;
    }
    else if(code == 7)
    {
        ldprintf("Module loader", LOG_ERR, "Given modules ELF version is not compatible with version loader can handle (%d)", EV_CURRENT);
        return 0;
    }
    else if(code == 8)
    {
        ldprintf("Module loader", LOG_ERR, "Given module does not support %s architecture.", KERNEL_ARCH);
        return 0;
    }
    else if(code == 9)
    {
        ldprintf("Module loader", LOG_ERR, "Given module is not built as a relative object");
        return 0;
    }
    ldprintf("Module loader", LOG_ERR, "Given module is not supported because of unknown reason");
    return -1;
}

/*
* Module loading Steps:
* Load the object to the memory
* Verify that object is elf
* Offset the address in section headers by the address of loaded object
* If the section header is SHT_NOBITS then allocate memory and initialize it to zero
* Find symtab, strtab and shstrtab from section headers
* Resolve the kernel symbols found in strtab
* Offset the address in symbols by the address of the section they reference
* Find the metadata in image
* Relocate the necessary address in relocation tables
* Parse the arguments
* Execute the function given as the initialization function in metadata
*/

int load_module(char** argv)
{
    if(validate(argv) != 1)
        return -1;
    if(validate(argv[0]) != 1)
        return -1;

    if(modules == NULL)
        modules = list_create();

    FILE* f = file_open(argv[0], OPEN_RDONLY);

    if(f == NULL)
    {
        ldprintf("Module loader", LOG_ERR, "Given module does not exist. Aborting");
        return -1;
    }


    uint32_t mod_sz = vfs_getFileSize(f);
    
    uint8_t* buf = zalloc(mod_sz);
    vfs_read(f, 0, mod_sz, buf);

    elf_header_t* head = buf;
    vfs_close(f);

    if(check_module(buf, mod_sz) != 1)
    {
        ldprintf("Module loader", LOG_ERR, "Module is not supported. Check kernel log for information.");
        free(buf);
        return -1;
    }

    elf_section_header_t* shdr = (buf + (head->e_shoff));

    // Load and update section headers in module
    for(uint32_t i = 0; i < head->e_shnum; i++)
    {
        if(shdr->sh_type == SHT_NOBITS)
        {
            shdr[i].sh_addr = module_alloc(shdr[i].sh_size, 1);
            memset(shdr[i].sh_addr, 0, shdr[i].sh_size);
            __K_MLDP__("%d 0x%x NOBITS", i, shdr[i].sh_addr);
        }
        else
        {
            shdr[i].sh_addr = (buf + shdr[i].sh_offset);
            __K_MLDP__("%d 0x%x UNKNOWN", i, shdr[i].sh_addr);
        }
    }

    module_info_t* info = NULL;
    module_entry_t* entry = ZALLOC_TYPES(module_entry_t);

    entry->ptr = buf;
    entry->mod_size = mod_sz;

    entry->symbols = list_create();
    entry->dependencies = list_create();

    char* strtab = NULL;
    char* shstrtab = shdr[head->e_shstrndx].sh_addr;

    int moddeps_shndx = 0;

    for(uint32_t i = 0; i < head->e_shnum; i++)
    {
        if(strcmp((&(shstrtab[shdr[i].sh_name])), "moddeps") == 0)
        {
            moddeps_shndx = i;
            break;
        }
    }

    uint32_t strtab_sz = 0;

    elf_sym_t* symtab = NULL;
    uint32_t nsyms = 0;

    for(uint32_t i = 0; i < head->e_shnum; i++)
    {
        if(shdr[i].sh_type != SHT_SYMTAB)
            continue;

        __K_MLDP__("Symtab found in address 0x%x with %d entries", shdr[i].sh_addr, (shdr[i].sh_size/shdr[i].sh_entsize));

        strtab = (char*)shdr[shdr[i].sh_link].sh_addr;
        strtab_sz = shdr[shdr[i].sh_link].sh_size;

        symtab = shdr[i].sh_addr;
        nsyms = (shdr[i].sh_size/shdr[i].sh_entsize);
    }

    if(validate(symtab) != 1)
        return -1;

    if(validate(strtab) != 1)
        return -1;

    if(nsyms == 0)
        return -1;
#if MODULE_LOADER_NO_EXTRA_INFO
#else
    serialprintf("\n\nNd Value    Size     Type    Bind    Ndx      Name\n");
#endif
    for(uint32_t s = 0; s < nsyms; s++)
    {
#if MODULE_LOADER_NO_EXTRA_INFO
#else
        if(symtab[s].st_info == STT_SECTION)
        {
            serialprintf("%02d %08x %08d %s %s %08d %s\n", s, symtab[s].st_value, symtab[s].st_size, get_stt(&symtab[s]), get_scope(&symtab[s]), (symtab[s].st_shndx), &(shstrtab[shdr[symtab[s].st_shndx].sh_name]));
        }
        else
        {
            serialprintf("%02d %08x %08d %s %s %08d %s\n", s, symtab[s].st_value, symtab[s].st_size, get_stt(&symtab[s]), get_scope(&symtab[s]), (symtab[s].st_shndx), &(strtab[symtab[s].st_name]));
        }
#endif

        if(symtab[s].st_shndx > 0 && symtab[s].st_shndx < SHN_LOPROC) // Local symbol
        {
            symtab[s].st_value += shdr[symtab[s].st_shndx].sh_addr;
            if(symtab[s].st_info == STT_SECTION)
                add_sym_to_modentry(strdup(&(shstrtab[shdr[symtab[s].st_shndx].sh_name])), symtab[s].st_value, symtab[s].st_size, ELF32_ST_TYPE(symtab[s].st_info), entry);
            else
                add_sym_to_modentry(strdup(&(strtab[symtab[s].st_name])), symtab[s].st_value, symtab[s].st_size, ELF32_ST_TYPE(symtab[s].st_info), entry);

            if(symtab[s].st_shndx == moddeps_shndx)
            {
                char* dep = symtab[s].st_value;
                if(list_contain_str(entry->dependencies, dep) != -1)
                    continue;
                list_push(entry->dependencies, strdup(dep));
                serialprintf("Module %s depends on %s\n", argv[0], dep);
                if(!module_isLoaded(dep))
                {
                    ldprintf("Module loader", LOG_ERR, "Module %s depends on module %s which is not loaded. Aborting", argv[0], dep);
                    list_destroy(entry->symbols);
                    list_destroy(entry->dependencies);
                    free(entry->ptr);
                    free(entry);
                    return -1;
                }
                else
                {
                    module_entry_t* me = get_module_entry_by_name(dep);
                    if(validate(me) != 1)
                        return -1;

                    me->depended_by++;
                }
            }

        }
        else if(symtab[s].st_shndx == SHN_UNDEF) // Kernel symbol
        {
            if(strcmp(&(strtab[symtab[s].st_name]), "") == 0)
                continue;

            symbol_t* sym = get_kernel_symbol_by_name(&(strtab[symtab[s].st_name]));
            if(sym == NULL)
            {
                sym = get_symbol_from_modules(&(strtab[symtab[s].st_name]));
                if(sym == NULL)
                {
                    ldprintf("Module loader", LOG_ERR, "Cannot resolve symbol named %s from the list of kernel and module symbols", &(strtab[symtab[s].st_name]));
                    kernel_panic("Cannot resolve kernel symbols present in module");
                }
            }
            symtab[s].st_value = (uintptr_t)sym->addr;
            symtab[s].st_size = sym->size;
            symtab[s].st_info = sym->type;
        }

        if(symtab[s].st_name && (strcmp(&(strtab[symtab[s].st_name]), "metadata") == 0))
        {
            info = symtab[s].st_value;
            entry->info = info;
        }
    }

    if(info == NULL)
    {
        ldprintf("Module loader", LOG_ERR, "Module %s does not contain metadata. Aborting", argv[0]);
        free(entry->ptr);
        free(entry);
        return -1;
    }

    if(info->magic != MODULE_METADATA_MAGIC)
    {
        ldprintf("Module loader", LOG_ERR, "Metadata of module %s contains invalid magic [0x%08x]", info->name, info->magic);
        free(entry->ptr);
        free(entry);
        return -1;
    }

    if(module_isLoaded(info->name))
    {
        ldprintf("Module loader", LOG_ERR, "Module %s currently loaded. Aborting", info->name);
        list_destroy(entry->dependencies);
        list_destroy(entry->symbols);
        free(entry->ptr);
        free(entry);
        return -1;
    }

    entry->depended_by = 0;

    for(uint32_t i = 0; i < head->e_shnum; i++)
    {
        if(shdr[i].sh_type != SHT_REL)
            continue;

        elf_rel_t* rel_table = shdr[i].sh_addr;
        uint32_t rel_entries = (shdr[i].sh_size/shdr[i].sh_entsize);

        for(uint32_t j = 0; j < rel_entries; j++)
        {
            elf_sym_t* sym = &(symtab[ELF32_R_SYM(rel_table[j].r_info)]);
            elf_section_header_t* rshdr = &(shdr[shdr[i].sh_info]);

            uint32_t* ptr = NULL;

            uint32_t A = 0;         // Addend
            uint32_t P = 0;         // Place
            uint32_t S = 0;         // Symbol

            if(ELF32_ST_TYPE(sym->st_info) == STT_SECTION)
            {
                __K_MLDP__("Relocating Section \"%s\"", &(shstrtab[shdr[sym->st_shndx].sh_name]));
                ptr = (void*)(rel_table[j].r_offset + rshdr->sh_addr);
                A = *ptr;
                P = (uintptr_t)ptr;
                S = shdr[sym->st_shndx].sh_addr;
            }
            else
            {
                char* name = &(strtab[sym->st_name]);
                __K_MLDP__("Relocating %s \"%s\"", get_stt(sym), name);

                ptr = (void*)(rel_table[j].r_offset + rshdr->sh_addr);
                A = *ptr;
                P = (uintptr_t)ptr;

                if(!get_module_symbol_by_name(name, symtab, strtab, nsyms))
                {
                    if(get_kernel_symbol_by_name(name))
                    {
                        symbol_t* symbol = get_kernel_symbol_by_name(name);
                        S = (uintptr_t)symbol->addr;
                    }
                    else
                    {
                        ldprintf("Module loader", LOG_ERR, "Cannot find symbol %s used by module. Aborting", name);
                        list_destroy(entry->dependencies);
                        list_destroy(entry->symbols);
                        free(entry->ptr);
                        free(entry);
                        return -1;
                    }
                }
                else
                {
                    elf_sym_t* symbol = get_module_symbol_by_name(name, symtab, strtab, nsyms);
                    S = (uintptr_t)(symbol->st_value);
                }

            }

            switch(ELF32_R_TYPE(rel_table[j].r_info))
            {
            case R_386_32:
                *ptr = A + S;
                __K_MLDP__("'R_386_32' 0x%x = 0x%x + 0x%x", *ptr, A, S);
                break;
            case R_386_PC32:
                *ptr = A + S - P;
                __K_MLDP__("'R_386_PC32' 0x%x = 0x%x + 0x%x - 0x%x", *ptr, A, S, P);
                break;
            default:
                ldprintf("Module loader", LOG_ERR, "Unsupported reallocation %d found", ELF32_R_TYPE(rel_table[j].r_info));
                list_destroy(entry->dependencies);
                list_destroy(entry->symbols);
                free(entry->ptr);
                free(entry);
                return -1;
                break;
            };
        }
    }

    list_push(modules, entry);
    int argc = 0;
	for (char ** aa = argv; *aa; ++aa) ++argc;

    symbol_t* init_func_sym = get_symbol_from_modulelist(info->init_name, entry);
    module_init init = init_func_sym->addr;

    ldprintf("Module loader", LOG_INFO, "Module %s successfully loaded.", info->name);
    //update_module_kfs_entry();
    return (init)(argc, argv);
}

int unload_module(char* name)
{
    if(validate(modules) != 1)
        return -1;

    module_entry_t* e = NULL;

    listnode_t* current_module_node = NULL;

    if(!module_isLoaded(name))
        return -1;

    foreach(k, modules)
    {
        e = k->val;
        if(strcmp(e->info->name, name) == 0)
        {
            current_module_node = k;
            break;
        }
    }

    if(e->depended_by != 0)
    {
        ldprintf("Module unloader", LOG_ERR, "Module %s is currently used by other modules. Please unload the depended modules", name);
        return -1;
    }

    foreach(l, e->dependencies)
    {
        char* dep_name = l->val;
        module_entry_t* me = get_module_entry_by_name(dep_name);
        if(validate(me) != 1)
            kernel_panic("Module is currently depended on a module which is not loaded. How?");

        me->depended_by--;
    }

    symbol_t* fini_func_sym = get_symbol_from_modulelist(e->info->fini_name, e);
    module_fini fini = fini_func_sym->addr;

    int ret = fini();

    list_remove_node(modules, current_module_node);
    list_destroy(e->dependencies);
    list_destroy(e->symbols);
    free(e->ptr);
    free(e);

    if(ret == 0)
        ldprintf("Module unloader", LOG_INFO, "Module %s successfully unloaded.", name);

    //update_module_kfs_entry();

    return ret;
}

int force_unload_module(char* name)
{
    if(validate(modules) != 1)
        return -1;

    module_entry_t* e = NULL;

    listnode_t* current_module_node = NULL;

    if(!module_isLoaded(name))
        return -1;

    foreach(k, modules)
    {
        e = k->val;
        if(strcmp(e->info->name, name) == 0)
        {
            current_module_node = k;
            break;
        }
    }

    symbol_t* fini_func_sym = get_symbol_from_modulelist(e->info->fini_name, e);
    module_fini fini = fini_func_sym->addr;

    int ret = fini();

    list_remove_node(modules, current_module_node);
    list_destroy(e->dependencies);
    list_destroy(e->symbols);
    free(e->ptr);
    free(e);

    if(ret == 0)
        ldprintf("Module unloader", LOG_INFO, "Module %s successfully unloaded.", name);

    return ret;
}

void print_list_modules()
{
    printf("Depended By  Address  Name\n");
    foreach(l, modules)
    {
        module_entry_t* e = l->val;
        printf("%02d           %08x %s\n", e->depended_by, e->ptr, e->info->name);
    }
}

int module_stopall()
{
    if(validate(modules) != 1)
        return -1;
    foreach(l, modules)
    {
        module_entry_t* e = l->val;

        force_unload_module(e->info->name);
    }
    return 0;
}