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

#define SHN_UNDEF	0
#define SHN_LORESERVE	0xff00
#define SHN_LOPROC	0xff00
#define SHN_BEFORE	0xff00

#define SHN_AFTER	0xff01

#define SHN_HIPROC	0xff1f
#define SHN_LOOS	0xff20
#define SHN_HIOS	0xff3f
#define SHN_ABS		0xfff1
#define SHN_COMMON	0xfff2
#define SHN_XINDEX	0xffff
#define SHN_HIRESERVE	0xffff

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

#define STT_NOTYPE	0		/* Symbol type is unspecified */
#define STT_OBJECT	1		/* Symbol is a data object */
#define STT_FUNC	2		/* Symbol is a code object */
#define STT_SECTION	3		/* Symbol associated with a section */
#define STT_FILE	4		/* Symbol's name is file name */
#define STT_COMMON	5		/* Symbol is a common data object */
#define STT_TLS		6		/* Symbol is thread-local data object*/
#define	STT_NUM		7		/* Number of defined types.  */
#define STT_LOOS	10		/* Start of OS-specific */
#define STT_GNU_IFUNC	10		/* Symbol is indirect code object */
#define STT_HIOS	12		/* End of OS-specific */
#define STT_LOPROC	13		/* Start of processor-specific */
#define STT_HIPROC	15		/* End of processor-specific */

#define R_386_NONE	   0
#define R_386_32	   1
#define R_386_PC32	   2
#define R_386_GOT32	   3
#define R_386_PLT32	   4
#define R_386_COPY	   5
#define R_386_GLOB_DAT	   6
#define R_386_JMP_SLOT	   7
#define R_386_RELATIVE	   8
#define R_386_GOTOFF	   9
#define R_386_GOTPC	   10
#define R_386_32PLT	   11
#define R_386_TLS_TPOFF	   14
#define R_386_TLS_IE	   15
#define R_386_TLS_GOTIE	   16
#define R_386_TLS_LE	   17
#define R_386_TLS_GD	   18
#define R_386_TLS_LDM	   19
#define R_386_16	   20
#define R_386_PC16	   21
#define R_386_8		   22
#define R_386_PC8	   23
#define R_386_TLS_GD_32	   24
#define R_386_TLS_GD_PUSH  25
#define R_386_TLS_GD_CALL  26
#define R_386_TLS_GD_POP   27
#define R_386_TLS_LDM_32   28
#define R_386_TLS_LDM_PUSH 29
#define R_386_TLS_LDM_CALL 30
#define R_386_TLS_LDM_POP  31
#define R_386_TLS_LDO_32   32
#define R_386_TLS_IE_32	   33
#define R_386_TLS_LE_32	   34
#define R_386_TLS_DTPMOD32 35
#define R_386_TLS_DTPOFF32 36
#define R_386_TLS_TPOFF32  37
#define R_386_SIZE32       38
#define R_386_TLS_GOTDESC  39
#define R_386_TLS_DESC_CALL 40
#define R_386_TLS_DESC     41
#define R_386_IRELATIVE	   42
#define R_386_GOT32X	   43
#define R_386_NUM	   44

#define ELF32_R_SYM(val)		((val) >> 8)
#define ELF32_R_TYPE(val)		((val) & 0xff)
#define ELF32_R_INFO(sym, type)		(((sym) << 8) + ((type) & 0xff))

#define ELF32_ST_BIND(val)		(((unsigned char) (val)) >> 4)
#define ELF32_ST_TYPE(val)		((val) & 0xf)
#define ELF32_ST_INFO(bind, type)	(((bind) << 4) + ((type) & 0xf))

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

typedef struct elf_rela
{
  Elf32_Addr	r_offset;
  Elf32_Word	r_info;
  Elf32_Sword	r_addend;
} elf_rela_t;

typedef struct elf_rel_struct
{
  Elf32_Addr	r_offset;
  Elf32_Word	r_info;
} elf_rel_t;

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

typedef struct
{
    Elf32_Word    st_name;
    Elf32_Addr    st_value;
    Elf32_Word    st_size;
    unsigned char st_info;
    unsigned char st_other;
    Elf32_Half    st_shndx;
} elf_sym_t;

void load_elf();

#endif /*__ELF_LOADER_H__*/