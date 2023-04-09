#include "elf_loader.h"
#include "process.h"


int valid_elf(elf_header_t * elf_head)
{
    if(elf_head->e_ident[EI_MAG0] != ELFMAG0)
        return 0;
    if(elf_head->e_ident[EI_MAG1] != ELFMAG1)
        return 9;
    if(elf_head->e_ident[EI_MAG2] != ELFMAG2)
        return 0;
    if(elf_head->e_ident[EI_MAG3] != ELFMAG3)
        return 0;

    if(elf_head->e_ident[EI_CLASS] != ELFCLASS32)
        return 0;
    if(elf_head->e_ident[EI_DATA] != ELFDATA2LSB)
        return 0;
    if(elf_head->e_ident[EI_VERSION] != EV_CURRENT)
        return 0;
    if(elf_head->e_machine != EM_386)
        return 0;
    if(elf_head->e_type != ET_REL && elf_head->e_type != ET_EXEC)
        return 0;
    return 1;
}

void load_elf()
{
    printf("[ELF_LOADER] Loading the ELF file.\n");
    uint32_t seg_begin, seg_end;
    char * filename = current_process->filename;
    current_process->state = TASK_LOADING;
    FILE * f = file_open(filename, 0);
    if(!f)
    {
        printf("[ELF_LOADER] Given file does not exists\n");
        exit(10);
    }
    uint32_t size = vfs_getFileSize(f);
    void * file = kmalloc(size);
    vfs_read(f, 0, size, file);
    elf_header_t * head = file;
    elf_program_header_t * prgm_head = (void*)head + head->e_phoff;

    if(!valid_elf(head))
    {
        printf("[ELF_LOADER] Invalid/Unsupported elf executable %s\n", filename);
        exit(10);
    }

    for(uint32_t i = 0; i < head->e_phnum; i++)
    {
        if(prgm_head->p_type == PT_LOAD)
        {
            seg_begin = prgm_head->p_vaddr;
            seg_end= seg_begin + prgm_head->p_memsz;
            alloc_region(current_process->page_dir, seg_begin, seg_end, 0, 0, 1);
            memcpy((void*)seg_begin, file + prgm_head->p_offset, prgm_head->p_filesz);
            memset((void*)(seg_begin + prgm_head->p_filesz), 0, prgm_head->p_memsz - prgm_head->p_filesz);
            if(prgm_head->p_flags == PF_X + PF_R + PF_W || prgm_head->p_flags == PF_X + PF_R)
            {
                current_process->regs.eip = head->e_entry;
                serialprintf("%x\n", current_process->regs.eip);
            }
        }
        prgm_head++;
    }

    printf("[ELF_LOADER] Given ELF file loaded. Executing...\n");
    alloc_region(current_process->page_dir, 0x08048054, 0x08048054 + 160, 0, 0, 1);
    alloc_page(current_process->page_dir, 0xC0000000 - 0x1000, 0, 0, 1);
    current_process->regs.esp = 0xC0000000;
    current_process->regs.ebp = current_process->regs.ebp;
    current_process->type = TASK_TYPE_USER;
    current_process->state = TASK_RUNNING;
    current_process->entrypoint = current_process->regs.eip;
    SYSCALL_CHANGE_PROCESS(current_process->pid)
}