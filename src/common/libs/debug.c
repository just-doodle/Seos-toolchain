#include "debug.h"
#include "elf_loader.h"
#include "process.h"
#include "logdisk.h"
#include "mount.h"
#include "kernelfs.h"
#include "modules.h"

symbol_t* kernel_symbols = NULL;
uint32_t n_kernel_symbols = 0;

void kernel_atCrash()
{
    module_stopall();
    logdisk_dump("/kernel.log");
}

void kernel_panic(const char* message)
{
    ldprintf("KERNEL PANIC", LOG_ERR, "%s", message);
    logdisk_change_policy(LOG_OFF);
    kernel_atCrash();
    while(1);
}

void kernel_panic_noHalt(const char* message)
{
    ldprintf("KERNEL PANIC", LOG_ERR, "%s", message);
    logdisk_change_policy(LOG_OFF);
    kernel_atCrash();
}

int validate(void* ptr)
{
    if(ptr == NULL)
        return 0;
    if(virt2phys(kernel_page_dir, ptr) == NULL)
    {
        if(current_process != NULL && current_process->page_dir != NULL)
        {
            if(virt2phys(current_process->page_dir, ptr) == NULL)
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }

    return 1;
}

void stack_trace(uint32_t maxframes)
{
    stackframe_t* stk;
    ASM_FUNC("movl %%ebp,%0" : "=r"(stk) ::);
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

    if(kernel_symbols != NULL)
    {
        for(int i = 0; i < n_kernel_symbols; i++)
        {
            sym = &(kernel_symbols[i]);
            uint32_t offset = get_offset(addr, sym);
            if(offset)
                return (symoff_t){sym->name, offset};
        }
    }

    return get_symoff_from_modules(addr);


	return (symoff_t){"????", 0};
}

void load_kernel_symbols(struct multiboot_tag_elf_sections* info)
{
    uint32_t shndx = info->shndx;
    uint32_t num = info->num;
    uint32_t size = info->size;
    uint8_t* shdr_buf = info->sections;

    alloc_region(kernel_page_dir, shdr_buf, shdr_buf + (num*size), 1, 1, 1);

    elf_section_header_t* shdr = (elf_section_header_t*)info->sections;

    uint32_t sym_num = 0;
    elf_sym_t* symtab = NULL;

    char* strtab = NULL;

    elf_section_header_t* sym_shdr = NULL;
    elf_section_header_t* str_shdr = NULL;

    for(uint32_t i = 0; i < num; i++)
    {
        if(shdr[i].sh_type == SHT_SYMTAB)
        {
            symtab = shdr[i].sh_addr;
            alloc_region(kernel_page_dir, symtab, symtab + shdr[i].sh_size, 1, 1, 1);
            sym_num = (shdr[i].sh_size / sizeof(elf_sym_t));
            strtab = shdr[shdr[i].sh_link].sh_addr;
            alloc_region(kernel_page_dir, strtab, strtab + shdr[shdr[i].sh_link].sh_size, 1, 1, 1);
            sym_shdr = &shdr[i];
            str_shdr = &(shdr[shdr[i].sh_link]);

        }
    }

    // ASSERT(symtab == NULL && "Kernel symbols not loaded");

    uint32_t func_num = (sym_shdr->sh_size/sym_shdr->sh_entsize);
    ldprintf("KERNEL", LOG_INFO, "Found %d symbols", func_num);

    kernel_symbols = zalloc(sizeof(symbol_t) * func_num);

    uint32_t j = 0;

    char* buf = zalloc((func_num*256)+512);

    for(uint32_t i = 0; i < func_num; i++)
    {
        if(symtab[i].st_info & STT_FUNC)
        {
            kernel_symbols[j].addr = symtab[i].st_value;
            kernel_symbols[j].size = symtab[i].st_size;
            kernel_symbols[j].name = strdup(&(strtab[symtab[i].st_name]));
            kernel_symbols[j].type = ELF32_ST_TYPE(symtab[i].st_info);
            sprintf(buf+strlen(buf), "%08x %s\n", kernel_symbols[j].addr, kernel_symbols[j].name);
            j++;
        }
    }

    n_kernel_symbols = j;
    kernelfs_addcharf("/proc", "kallsyms", buf);
    free(buf);
}

symbol_t* get_kernel_symbol_by_name(char* name)
{
    if(kernel_symbols == NULL)
        return NULL;

    for(int i = 0; i < n_kernel_symbols; i++)
    {
        symbol_t* sym = &(kernel_symbols[i]);
        if(strcmp(name, sym->name) == 0)
        {
            return sym;
        }
    }
    
    return NULL;
}

#define FUNCTION_ARG_CHAR 0
#define FUNCTION_ARG_CHAR_P 1
#define FUNCTION_ARG_U8 2
#define FUNCTION_ARG_U16 3
#define FUNCTION_ARG_U32 4
#define FUNCTION_ARG_U8_P 6
#define FUNCTION_ARG_U16_P 7
#define FUNCTION_ARG_U32_P 8
#define FUNCTION_ARG_S8 9
#define FUNCTION_ARG_S16 10
#define FUNCTION_ARG_S32 11
#define FUNCTION_ARG_S8_P 12
#define FUNCTION_ARG_S16_P 13
#define FUNCTION_ARG_S32_P 14
#define FUNCTION_ARG_POINTER 15
#define FUNCTION_ARG_VOID_PTR 16

typedef struct debug_function_argument_struct
{
    uint32_t type;
    uint32_t val;   
}debug_function_arg_t;

uint32_t call_module_function_helper(char* module, char* function, char* fmt, va_list list)
{

    ldprintf("function caller", LOG_DEBUG, "Calling %s from %s", function, module);

    symbol_t* sym = NULL;

    if(strcmp("__KERNEL_SYMTAB__//", module) == 0)
    {
        sym = get_kernel_symbol_by_name(function);
    }
    else
    {
        if(!module_isLoaded(module))
            return -1;
        module_entry_t* mentry = get_module_entry_by_name(module);
        if(validate(mentry) != 1)
            return -1;
        foreach(l, mentry->symbols)
        {
            symbol_t* s = l->val;
            if(strcmp(function, s->name) == 0)
            {
                sym = s;
                break;
            }
        }
    }

    if(validate(sym) != 1)
    {
        ldprintf("function caller", LOG_ERR, "Given function %s does not exist in symbol table", function);
        return -1;
    }

    uint32_t ret = 0;
    list_t* args = list_create();
    char c = '\0';
    uint32_t size;
    char width_str[10];
    int size_override = 0;
    int i = 0;

    while ((c = *fmt++) != 0)
    {
        if(validate(fmt) != 1)
            break;
        if (c == '%')
        {
            c = *fmt++;
            // serialprintf("Got fmt %c\n", c);
            switch(c)
            {
            case '0':
            {
                size_override = 1;
                i = 0;
                c = *fmt;
                while (!is_fmt_letter(c))
                {
                    width_str[i++] = c;
                    fmt++;
                    c = *fmt;
                }
                width_str[i] = 0;
                fmt++;
                size = atoi(width_str);
                serialprintf("Got fmt %c\n", *(fmt-1));
                switch(*(fmt-1))
                {    
                case 'u':
                {
                    if(size_override)
                    {
                        switch(size)
                        {
                        case 8:
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_U8;
                            a->val = va_arg(list, int);
                            list_push(args, a);
                        }break;
                        case 16:
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_U16;
                            a->val = va_arg(list, int);
                            list_push(args, a);
                        }break;
                        case 32:
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_U32;
                            a->val = va_arg(list, uint32_t);
                            list_push(args, a);
                        }break;
                        default:
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_U8;
                            a->val = va_arg(list, int);
                            list_push(args, a);
                        }break;
                        };
                    }
                    else
                    {
                        debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                        a->type = FUNCTION_ARG_U32;
                        a->val = va_arg(list, uint32_t);
                        list_push(args, a);
                    }
                }break;
                case 's':
                {
                    if(size_override)
                    {
                        switch(size)
                        {
                        case 8:
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_S8;
                            a->val = va_arg(list, int);
                            list_push(args, a);
                        }break;
                        case 16:
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_S16;
                            a->val = va_arg(list, int);
                            list_push(args, a);
                        }break;
                        case 32:
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_S32;
                            a->val = va_arg(list, int32_t);
                            list_push(args, a);
                        }break;
                        default:
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_S8;
                            a->val = va_arg(list, int);
                            list_push(args, a);
                        }break;
                        };
                    }
                    else
                    {
                        debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                        a->type = FUNCTION_ARG_S32;
                        a->val = va_arg(list, int32_t);
                        list_push(args, a);
                    }
                }break;
                case 'p':
                {
                    char t = *fmt++;
                    switch(t)
                    {
                    case 'u':
                    {
                        if(size_override)
                        {
                            switch(size)
                            {
                            case 8:
                            {
                                debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                                a->type = FUNCTION_ARG_U8_P;
                                a->val = va_arg(list, uint8_t*);
                                list_push(args, a);
                            }break;
                            case 16:
                            {
                                debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                                a->type = FUNCTION_ARG_U16_P;
                                a->val = va_arg(list, uint16_t*);
                                list_push(args, a);
                            }break;
                            case 32:
                            {
                                debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                                a->type = FUNCTION_ARG_U32_P;
                                a->val = va_arg(list, uint32_t*);
                                list_push(args, a);
                            }break;
                            default:
                            {
                                debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                                a->type = FUNCTION_ARG_U8_P;
                                a->val = va_arg(list, uint8_t*);
                                list_push(args, a);
                            }break;
                            };
                        }
                        else
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_U32_P;
                            a->val = va_arg(list, uint32_t*);
                            list_push(args, a);
                        }
                    }break;
                    case 's':
                    {
                        if(size_override)
                        {
                            switch(size)
                            {
                            case 8:
                            {
                                debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                                a->type = FUNCTION_ARG_S8_P;
                                a->val = va_arg(list, int8_t*);
                                list_push(args, a);
                            }break;
                            case 16:
                            {
                                debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                                a->type = FUNCTION_ARG_S16_P;
                                a->val = va_arg(list, int16_t*);
                                list_push(args, a);
                            }break;
                            case 32:
                            {
                                debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                                a->type = FUNCTION_ARG_S32_P;
                                a->val = va_arg(list, int32_t*);
                                list_push(args, a);
                            }break;
                            default:
                            {
                                debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                                a->type = FUNCTION_ARG_S8_P;
                                a->val = va_arg(list, int8_t*);
                                list_push(args, a);
                            }break;
                            };
                        }
                        else
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_S32_P;
                            a->val = va_arg(list, int32_t*);
                            list_push(args, a);
                        }
                    }break;
                    case 'c':
                    {
                        debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                        a->type = FUNCTION_ARG_CHAR_P;
                        a->val = va_arg(list, char*);
                        list_push(args, a);
                    }break;
                    default:
                    {
                        debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                        a->type = FUNCTION_ARG_POINTER;
                        a->val = va_arg(list, void*);
                        list_push(args, a);
                    }break;
                    };
                }break;
                };
            }break;
            case 'u':
            {
                if(size_override)
                {
                    switch(size)
                    {
                    case 8:
                    {
                        debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                        a->type = FUNCTION_ARG_U8;
                        a->val = va_arg(list, int);
                        list_push(args, a);
                    }break;
                    case 16:
                    {
                        debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                        a->type = FUNCTION_ARG_U16;
                        a->val = va_arg(list, int);
                        list_push(args, a);
                    }break;
                    case 32:
                    {
                        debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                        a->type = FUNCTION_ARG_U32;
                        a->val = va_arg(list, uint32_t);
                        list_push(args, a);
                    }break;
                    default:
                    {
                        debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                        a->type = FUNCTION_ARG_U8;
                        a->val = va_arg(list, int);
                        list_push(args, a);
                    }break;
                    };
                }
                else
                {
                    debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                    a->type = FUNCTION_ARG_U32;
                    a->val = va_arg(list, uint32_t);
                    list_push(args, a);
                }
            }break;
            case 's':
            {
                if(size_override)
                {
                    switch(size)
                    {
                    case 8:
                    {
                        debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                        a->type = FUNCTION_ARG_S8;
                        a->val = va_arg(list, int);
                        list_push(args, a);
                    }break;
                    case 16:
                    {
                        debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                        a->type = FUNCTION_ARG_S16;
                        a->val = va_arg(list, int);
                        list_push(args, a);
                    }break;
                    case 32:
                    {
                        debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                        a->type = FUNCTION_ARG_S32;
                        a->val = va_arg(list, int32_t);
                        list_push(args, a);
                    }break;
                    default:
                    {
                        debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                        a->type = FUNCTION_ARG_S8;
                        a->val = va_arg(list, int);
                        list_push(args, a);
                    }break;
                    };
                }
                else
                {
                    debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                    a->type = FUNCTION_ARG_S32;
                    a->val = va_arg(list, int32_t);
                    list_push(args, a);
                }
            }break;
            case 'p':
            {
                char t = *fmt++;
                switch(t)
                {
                case 'u':
                {
                    if(size_override)
                    {
                        switch(size)
                        {
                        case 8:
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_U8_P;
                            a->val = va_arg(list, uint8_t*);
                            list_push(args, a);
                        }break;
                        case 16:
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_U16_P;
                            a->val = va_arg(list, uint16_t*);
                            list_push(args, a);
                        }break;
                        case 32:
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_U32_P;
                            a->val = va_arg(list, uint32_t*);
                            list_push(args, a);
                        }break;
                        default:
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_U8_P;
                            a->val = va_arg(list, uint8_t*);
                            list_push(args, a);
                        }break;
                        };
                    }
                    else
                    {
                        debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                        a->type = FUNCTION_ARG_U32_P;
                        a->val = va_arg(list, uint32_t*);
                        list_push(args, a);
                    }
                }break;
                case 's':
                {
                    if(size_override)
                    {
                        switch(size)
                        {
                        case 8:
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_S8_P;
                            a->val = va_arg(list, int8_t*);
                            list_push(args, a);
                        }break;
                        case 16:
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_S16_P;
                            a->val = va_arg(list, int16_t*);
                            list_push(args, a);
                        }break;
                        case 32:
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_S32_P;
                            a->val = va_arg(list, int32_t*);
                            list_push(args, a);
                        }break;
                        default:
                        {
                            debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                            a->type = FUNCTION_ARG_S8_P;
                            a->val = va_arg(list, int8_t*);
                            list_push(args, a);
                        }break;
                        };
                    }
                    else
                    {
                        debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                        a->type = FUNCTION_ARG_S32_P;
                        a->val = va_arg(list, int32_t*);
                        list_push(args, a);
                    }
                }break;
                case 'c':
                {
                    debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                    a->type = FUNCTION_ARG_CHAR_P;
                    a->val = va_arg(list, char*);
                    list_push(args, a);
                }break;
                default:
                {
                    debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                    a->type = FUNCTION_ARG_POINTER;
                    a->val = va_arg(list, void*);
                    list_push(args, a);
                }break;
                };
            }break;
            case 'c':
            {
                debug_function_arg_t* a = zalloc(sizeof(debug_function_arg_t));
                a->type = FUNCTION_ARG_CHAR;
                a->val = va_arg(list, int);
                list_push(args, a);
            }break;
            };
        }
    }

    uint32_t argc = list_size(args);
    ldprintf("function caller", LOG_DEBUG, "LOOP exited with args %d", (argc));

    foreach(k, args)
    {
        debug_function_arg_t* a = k->val;
        ldprintf("function caller", LOG_DEBUG, "Got argument %x", a->val);
        ASM_FUNC("push %0" :: "r"(a->val));
    }

    ASM_FUNC("call *%1" : "=a"(ret) : "r"(sym->addr));
    ldprintf("function caller", LOG_DEBUG, "Function called");

    foreach(k, args)
    {
        ASM_FUNC("pop %%ecx"::);
        free(k->val);
    }

    list_destroy(args);
    return ret;
}

uint32_t call_module_function(char* module, char* function, char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    uint32_t ret = call_module_function_helper(module, function, fmt, args);
    va_end(args);
    return ret;
}

void backtrace()
{
    stackframe_t* stk;
    ASM_FUNC("movl %%ebp,%0" : "=r"(stk) ::);
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