#include "syscall.h"
#include "process.h"
#include "stat.h"
#include "filedescriptor.h"
#include "vesa.h"
#include "compositor.h"

void * syscall_table[MAX_SYSCALLS] = {
    text_putc, //0
    exit, //1
    attach_handler, //2
    fd_open, //3
    fd_close, //4
    fd_read, //5
    fd_write, //6
    fd_lseek, //7
    fstat, //8
    stat, //9
    vfs_unlink, //10
    getpid, //11
    kill, //12
    malloc, //13
    free, //14
    realloc, //15
    ksbrk, //16
    rand, //17
    sleep, //18
    execve, //19
    change_process, //20
    text_clear, //21
    get_args, //22
    create_window, //23
    window_display, //24
    pit_register, //25
    sleep, //26
    window_change_title, //27
};

void syscall_create_process(char* name, void* entrypoint)
{
    create_process_from_routine(name, entrypoint, TASK_TYPE_USER);
}

void syscall_vfs_read(vfs_node* file, syscall_rwo_t* options)
{
    vfs_read(file, options->offset, options->size, options->buffer);
}

void syscall_vfs_write(vfs_node* file, syscall_rwo_t* options)
{
    vfs_read(file, options->offset, options->size, options->buffer);
}

void syscall_handler(registers_t *reg)
{
    if(reg->eax >= MAX_SYSCALLS) return;
    void * system_api = syscall_table[reg->eax];
    int ret;
    //memcpy(&saved_context, reg, sizeof(registers_t));
    //serialprintf("SYSCALL 0x%02x\n", reg->eax);
    asm volatile (" \
    push %1; \
    push %2; \
    push %3; \
    push %4; \
    push %5; \
    call *%6; \
    pop %%ebx; \
    pop %%ebx; \
    pop %%ebx; \
    pop %%ebx; \
    pop %%ebx; \
    " : "=a" (ret) : "r" (reg->edi), "r" (reg->esi), "r" (reg->edx), "r" (reg->ecx), "r" (reg->ebx), "r" (system_api));

    reg->eax = ret;
}

void init_syscall()
{
    register_interrupt_handler(0x80, syscall_handler);
}