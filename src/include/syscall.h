#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "system.h"
#include "printf.h"
#include "vga_text.h"
#include "isr.h"
#include "vfs.h"

#define MAX_SYSCALLS 18

#define SYSCALL(ret, a, b, c, d) asm volatile("int $0x80": "=a"(ret) : "a"(a), "b"(b), "c"(c), "d"(d));
#define SYSCALL2(a, b, c, d) asm volatile("int $0x80": : "a"(a), "b"(b), "c"(c), "d"(d));

#define SYSCALL_PUTCHAR(c) SYSCALL2(0, c, 0, 0)
#define SYSCALL_EXIT(code)    SYSCALL2(1, code, 0, 0)
#define SYSCALL_ATTACH(handler)  SYSCALL2(2, handler, 0, 0)
#define SYSCALL_FILE_OPEN(filename, flags, ret) SYSCALL(ret, 3, filename, flags, 0)
#define SYSCALL_VFS_CLOSE(file) SYSCALL2(4, file, 0, 0)
#define SYSCALL_VFS_READ(file, options) SYSCALL2(5, file, options, 0)
#define SYSCALL_VFS_WRITE(file, options) SYSCALL2(6, file, options, 0)
#define SYSCALL_VFS_GETFILESIZE(file, size) SYSCALL(size, 7, file, 0, 0)
#define SYSCALL_VFS_CREATE(path, permissions) SYSCALL2(8, path, permissions, 0)
#define SYSCALL_MALLOC(ptr, size) SYSCALL(ptr, 9, size, 0, 0)
#define SYSCALL_FREE(ptr) SYSCALL2(10, ptr, 0, 0)
#define SYSCALL_REALLOC(ret_ptr, ptr, size) SYSCALL(ret_ptr, 11, ptr, size, 0)
#define SYSCALL_RAND(ret) SYSCALL(ret, 12, 0, 0, 0)
#define SYSCALL_WAIT(ms) SYSCALL2(13, ms, 0, 0)
#define SYSCALL_CREATE_PROCESS_FUN(name, routine) SYSCALL2(14, name, routine, 0)
#define SYSCALL_CHANGE_PROCESS(pid) SYSCALL2(15, pid, 0, 0)
#define SYSCALL_CLEAR SYSCALL2(16, 0, 0, 0)

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