#include "modules.h"

uint32_t module_mapper_addr = MODULE_ALLOCATION_ADDRESS;
uint32_t module_mapper_end = MODULE_ALLOCATION_END;

list_t* modules = NULL;

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

symbol_t* get_symbol_from_modulelist(char* name)
{
    if(validate(modules) != 1)
        return NULL;
    foreach(mods, modules)
    {
        module_entry_t* entry = mods->val;
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

void add_sym_to_modentry(char* name, uint32_t addr, uint32_t size, module_entry_t* me)
{
    symbol_t* s = ZALLOC_TYPES(symbol_t);
    s->name = name;
    s->addr = addr;
    s->size = size;

    if(me->symbols == NULL)
        me->symbols = list_create();

    list_push(me->symbols, s);
}

int load_module(char** argv)
{
    if(validate(argv) != 1)
        return -1;
    if(validate(argv[0]) != 1)
        return -1;

    if(modules == NULL)
        modules = list_create();

    FILE* f = file_open(argv[0], OPEN_RDONLY);
    uint32_t mod_sz = vfs_getFileSize(f);
    
    uint8_t* buf = zalloc(mod_sz);
    vfs_read(f, 0, mod_sz, buf);

    elf_header_t* head = buf;
    vfs_close(f);

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

    if(code != 0)
    {
        free(buf);
        ldprintf("Module loader", LOG_ERR, "Given module is not an elf file");
        return -1;
    }

    for(uint32_t i = 0; i < head->e_shnum; i++)
    {
        elf_section_header_t* shdr = (elf_section_header_t*)(buf + (head->e_shoff + (head->e_shentsize * i)));
        if(shdr->sh_type == SHT_NOBITS)
        {
            shdr->sh_addr = module_alloc(shdr->sh_size, 1);
            memset(shdr->sh_addr, 0, shdr->sh_size);
        }
        else
        {
            shdr->sh_addr = (buf + shdr->sh_offset);
            serialprintf("SHDR %d: 0x%x\n", i, shdr->sh_addr);
        }
    }

    module_info_t* mod_info = NULL;

    module_entry_t* mod_entry = ZALLOC_TYPES(module_entry_t);
    mod_entry->mod_size = mod_sz;
    mod_entry->ptr = buf;

    char* strtab;

    elf_sym_t* symtab;
    uint32_t sym_num = 0;

    for(uint32_t i = 0; i < head->e_shnum; i++)
    {
        elf_section_header_t* shdr = (elf_section_header_t*)(buf + (head->e_shoff + (head->e_shentsize * i)));
        if(shdr->sh_type != SHT_SYMTAB)
            continue;

        elf_section_header_t* str_shdr = (elf_section_header_t*)(buf + (head->e_shoff + (head->e_shentsize * shdr->sh_link)));
        strtab = (str_shdr->sh_addr);

        symtab = shdr->sh_addr;

        for(uint32_t s = 0; s < (shdr->sh_size/sizeof(elf_sym_t)); s++)
        {
            if(symtab[s].st_shndx > 0 && symtab[s].st_shndx < SHN_LOPROC) // Local symbol
            {
                elf_section_header_t* sym_shdr = (buf + (head->e_shoff + (head->e_shentsize * symtab[s].st_shndx)));
                symtab[s].st_value = symtab[s].st_value + sym_shdr->sh_addr;
                serialprintf("SYM: %s : 0x%x\n", &(strtab[symtab[s].st_name]), symtab[s].st_value);
            }
            else if(symtab[s].st_shndx == SHN_UNDEF) // Kernel symbol
            {
                serialprintf("STRTAB: 0x%x\n", strtab);
                serialprintf("SYMTAB: 0x%x\n", symtab);
                serialprintf("SYM: %s\n", &(strtab[symtab[s].st_name]));
                symtab[s].st_value = (uintptr_t)get_kernel_symbol_by_name(&(strtab[symtab[s].st_name]));
            }
            add_sym_to_modentry(strdup(&(strtab[symtab[s].st_name])), symtab[s].st_value, symtab[s].st_size, mod_entry);
            if(symtab[s].st_name && (strcmp(&(strtab[symtab[s].st_name]), "metadata") == 0))
            {
                mod_info = (void*)symtab[s].st_value;
                mod_entry->info = mod_info;
                serialprintf("MOD_INFO: 0x%x\n", mod_info);
            }
            sym_num++;
        }
    }

    if(mod_info == NULL)
    {
        ldprintf("Module loader", LOG_ERR, "Module %s does not contain metadata. Aborting", argv[0]);
        free(mod_entry->ptr);
        free(mod_entry);
        return -1;
    }

    for(uint32_t i = 0; i < head->e_shnum; i++)
    {
        elf_section_header_t* shdr = (buf + (head->e_shoff + (head->e_shentsize * i)));
        if(shdr->sh_type != SHT_REL)
            continue;

        elf_rel_t* rel_table = shdr->sh_addr;
        for(uint32_t rel = 0; rel < (shdr->sh_size / sizeof(elf_rel_t)); rel++)
        {
            elf_sym_t* sym = &(symtab[ELF32_R_SYM(rel_table[rel].r_info)]);
            elf_section_header_t* r_section = (buf + (head->e_shoff + (head->e_shentsize * shdr->sh_info)));

            uint32_t A = 0;
            uint32_t P = 0;
            uint32_t S = 0;
            uint32_t* ptr = NULL;

            if(ELF32_ST_TYPE(sym->st_info) == STT_SECTION)
            {
                elf_section_header_t* sh_dr = (buf + (head->e_shoff + (sym->st_shndx * head->e_shentsize)));
                ptr = (rel_table->r_offset + r_section->sh_addr);
                A = *ptr;
                P = (uint32_t)ptr;
                S = sh_dr->sh_addr;
            }
            else
            {
                char* name = &(strtab[sym->st_name]);
                ptr = (rel_table->r_offset + r_section->sh_addr);
                A = *ptr;
                P = (uintptr_t)ptr;
                ldprintf("Module loader", LOG_INFO, "Using symbol %s", name);
                if(!get_module_symbol_by_name(name, symtab, strtab, sym_num))
                {
                    if(get_kernel_symbol_by_name(name))
                    {
                        S = (uintptr_t)((symbol_t*)get_kernel_symbol_by_name(name))->addr;
                    }
                    else
                    {
                        ldprintf("Module loader", LOG_ERR, "Cannot find symbol %s", name);
                    }
                }
                else
                {
                    S = (uintptr_t)(get_module_symbol_by_name(name, symtab, strtab, sym_num)->st_value);
                }
            }

            switch(ELF32_R_TYPE(rel_table[rel].r_info))
            {
            case R_386_32:
                *ptr = A + S;
                serialprintf("0x%x = 0x%x + 0x%x\n", *ptr, A, S);
                break;
            case R_386_PC32:
                *ptr = A + S - P;
                serialprintf("0x%x = 0x%x + 0x%x - 0x%x\n", *ptr, A, S, P);
                break;
            default:
                ldprintf("Module loader", LOG_ERR, "Unsupported reallocation %d found", ELF32_R_TYPE(rel_table[rel].r_info));
                free(mod_entry->ptr);
                free(mod_entry);
                return -1;
                break;
            };
        }
    }

    list_push(modules, mod_entry);

    int argc = 0;
	for (char ** aa = argv; *aa; ++aa) ++argc;

    serialprintf("Got symbols:\n");
    module_init* init = NULL;
    foreach(g, mod_entry->symbols)
    {
        symbol_t* s = g->val;
        serialprintf("\t-%s: 0x%x 0x%x\n", s->name, s->addr, s->size);
        if (strcmp(s->name, "init") == 0)
        {
            init = 0xc1893310;
        }
    }

    xxd(mod_info, sizeof(module_info_t));
    ldprintf("Module loader", LOG_INFO, "Initializing module: %s", mod_info->name);
    return (*((module_init*)0x8312663d))(argc, argv);
}