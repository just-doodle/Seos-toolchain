#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "system.h"
#include "printf.h"
#include "vga_text.h"
#include "isr.h"
#include "vfs.h"

#include "user_syscall.h"

typedef struct syscall_rw_options
{
    uint32_t offset;
    uint32_t size;
    char* buffer;
}syscall_rwo_t;

void syscall_vfs_read(vfs_node* file, syscall_rwo_t* options);
void syscall_vfs_write(vfs_node* file, syscall_rwo_t* options);
void syscall_create_process(char* name, void* entrypoint);

void syscall_handler(registers_t* reg);
void init_syscall();

#endif /*__SYSCALL_H__*/