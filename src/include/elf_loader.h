#ifndef __ELF_LOADER_H__
#define __ELF_LOADER_H__

#include "system.h"
#include "syscall.h"
#include "string.h"
#include "kheap.h"
#include "vfs.h"
#include "paging.h"
#include "printf.h"

#define DEFAULT_ELF_LOAD_ADDR (void*)0x08048000

#define ELF_NIDENT 16

#define ELFMAG0      0x7F
#define ELFMAG1     'E'
#define ELFMAG2     'L'
#define ELFMAG3     'F'
#define ELFDATA2LSB (1)
#define ELFCLASS32  (1)
#define EM_386     (3)
#define EV_CURRENT (1)

#define SHN_UNDEF  (0x00)

#define PT_NULL             0
#define PT_LOAD             1
#define PT_DYNAMIC          2
#define PT_INTERP           3
#define PT_NOTE             4
#define PT_SHLIB            5
#define PT_PHDR             6
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7fffffff

#define PF_X    0x1
#define PF_W    0x2
#define PF_R    0x4

typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Word;
typedef int  Elf32_Sword;

enum Elf_Ident
{
    EI_MAG1     = 1,
    EI_MAG0     = 0,
    EI_MAG3     = 3,
    EI_MAG2     = 2,
    EI_DATA     = 5,
    EI_CLASS    = 4,
    EI_OSABI    = 7,
    EI_VERSION  = 6,
    EI_ABIVERSION   = 8,
    EI_PAD      = 9
};

enum Elf_Type
{
    ET_NONE     = 0,
    ET_REL      = 1,
    ET_EXEC     = 2 
};

enum ShT_Types
{
    SHT_NULL    = 0,
    SHT_PROGBITS= 1,
    SHT_SYMTAB  = 2,
    SHT_STRTAB  = 3,
    SHT_RELA    = 4,
    SHT_NOBITS  = 8,
    SHT_REL     = 9,
};

enum ShT_Attributes
{
    SHF_WRITE   = 0x01,
    SHF_ALLOC   = 0x02 
};


typedef struct elf_header
{
    uint8_t     e_ident[ELF_NIDENT];
    Elf32_Half  e_type;
    Elf32_Half  e_machine;
    Elf32_Word  e_version;
    Elf32_Addr  e_entry;
    Elf32_Off   e_phoff;
    Elf32_Off   e_shoff;
    Elf32_Word  e_flags;
    Elf32_Half  e_ehsize;
    Elf32_Half  e_phentsize;
    Elf32_Half  e_phnum;
    Elf32_Half  e_shentsize;
    Elf32_Half  e_shnum;
    Elf32_Half  e_shstrndx;
}elf_header_t;

typedef struct elf_section_header
{
    Elf32_Word  sh_name;
    Elf32_Word  sh_type;
    Elf32_Word  sh_flags;
    Elf32_Addr  sh_addr;
    Elf32_Off   sh_offset;
    Elf32_Word  sh_size;
    Elf32_Word  sh_link;
    Elf32_Word  sh_info;
    Elf32_Word  sh_addralign;
    Elf32_Word  sh_entsize;
}elf_section_header_t;

typedef struct elf_program_header
{
    Elf32_Word      p_type;
    Elf32_Off       p_offset;
    Elf32_Addr      p_vaddr;
    Elf32_Addr      p_paddr;
    Elf32_Word      p_filesz;
    Elf32_Word      p_memsz;
    Elf32_Word      p_flags;
    Elf32_Word      p_align;
}elf_program_header_t;

void load_elf();

#endif /*__ELF_LOADER_H__*/