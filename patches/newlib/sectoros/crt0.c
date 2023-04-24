#include <fcntl.h>

#define SYSCODE_PUTC 0x00
#define SYSCODE_EXIT 0x01
#define SYSCODE_ATTACH 0x02
#define SYSCODE_OPEN 0x03
#define SYSCODE_CLOSE 0x04
#define SYSCODE_READ 0x05
#define SYSCODE_WRITE 0x06
#define SYSCODE_LSEEK 0x07
#define SYSCODE_FSTAT 0x08
#define SYSCODE_STAT 0x09
#define SYSCODE_UNLINK 0x0A
#define SYSCODE_GETPID 0x0B
#define SYSCODE_KILL 0x0C
#define SYSCODE_MALLOC 0x0D
#define SYSCODE_FREE 0x0E
#define SYSCODE_REALLOC 0x0F
#define SYSCODE_SBRK 0x10
#define SYSCODE_RAND 0x11
#define SYSCODE_WAIT 0x12
#define SYSCODE_EXECVE 0x13
#define SYSCODE_CHANGE_PROCESS 0x14
#define SYSCODE_CLEAR 0x15
#define SYSCODE_GET_ARGS 0x16
#define SYSCODE_CHANGE_VIDEO_MODE 0x17
#define SYSCODE_COPY_FRAMEBUFFER 0x18
#define SYSCODE_PIT_REGISTER 0x19
#define SYSCODE_SLEEP 0x1A
#define SYSCODE_PUT_PIXEL 0x1B

#define SYSCALL(ret, a, b, c, d) asm volatile("int $0x80": "=a"(ret) : "a"(a), "b"(b), "c"(c), "d"(d));
#define SYSCALL2(a, b, c, d) asm volatile("int $0x80": : "a"(a), "b"(b), "c"(c), "d"(d));

#define SYSCALL_PUTC(c) SYSCALL2(SYSCODE_PUTC, c, 0, 0)
#define SYSCALL_EXIT(code)    SYSCALL2(SYSCODE_EXIT, code, 0, 0)
#define SYSCALL_ATTACH(handler)  SYSCALL2(SYSCODE_ATTACH, handler, 0, 0)
#define SYSCALL_OPEN(filename, flags, mode, fd) SYSCALL(fd, SYSCODE_OPEN, filename, flags, mode)
#define SYSCALL_CLOSE(fd, ret) SYSCALL(ret, SYSCODE_CLOSE, fd, 0, 0)
#define SYSCALL_READ(fd, ptr, len, ret) SYSCALL(ret, SYSCODE_READ, fd, ptr, len)
#define SYSCALL_WRITE(fd, ptr, len, ret) SYSCALL(ret, SYSCODE_WRITE, fd, ptr, len)
#define SYSCALL_LSEEK(fd, off, dir, ret) SYSCALL(ret, SYSCODE_LSEEK, fd, off, dir)
#define SYSCALL_FSTAT(fd, seos_stat, ret) SYSCALL(ret, SYSCODE_FSTAT, fd, seos_stat, 0)
#define SYSCALL_STAT(name, seos_stat, ret) SYSCALL(ret, SYSCODE_STAT, name, seos_stat, 0)
#define SYSCALL_GETPID(ret) SYSCALL(ret, SYSCODE_GETPID, 0, 0, 0)
#define SYSCALL_KILL(pid, sig, ret) SYSCALL(ret, SYSCODE_KILL, pid, sig, 0);
#define SYSCALL_MALLOC(ptr, size) SYSCALL(ptr, SYSCODE_MALLOC, size, 0, 0)
#define SYSCALL_FREE(ptr) SYSCALL2(SYSCALL_FREE, ptr, 0, 0)
#define SYSCALL_REALLOC(ret_ptr, ptr, size) SYSCALL(ret_ptr, SYSCODE_REALLOC, ptr, size, 0)
#define SYSCALL_SBRK(size, ret) SYSCALL(ret, SYSCODE_SBRK, size, 0, 0)
#define SYSCALL_RAND(ret) SYSCALL(ret, SYSCODE_RAND, 0, 0, 0)
#define SYSCALL_WAIT(ms) SYSCALL2(SYSCODE_WAIT, ms, 0, 0)
#define SYSCALL_EXECVE(name, argv, env) SYSCALL2(SYSCODE_EXECVE, name, argv, env)
#define SYSCALL_CHANGE_PROCESS(pid) SYSCALL2(SYSCODE_CHANGE_PROCESS, pid, 0, 0)
#define SYSCALL_CLEAR SYSCALL2(SYSCODE_CLEAR, 0, 0, 0)
#define SYSCALL_GET_ARGS(ret) SYSCALL(ret, SYSCODE_GET_ARGS, 0, 0, 0)
#define SYSCALL_CHANGE_VIDEO_MODE(width, height, bpp) SYSCALL2(SYSCODE_CHANGE_VIDEO_MODE, width, height, bpp);
#define SYSCALL_COPY_FRAMEBUFFER(fb) SYSCALL2(SYSCODE_COPY_FRAMEBUFFER, fb, 0, 0)
#define SYSCALL_GET_TICKS(ret) SYSCALL(ret, SYSCODE_GET_TICKS, 0, 0, 0)
#define SYSCALL_PIT_REGISTER(callback, seconds) SYSCALL2(SYSCODE_PIT_REGISTER, callback, seconds, 0)
#define SYSCALL_PUT_PIXEL(x, y, pixel) SYSCALL2(SYSCODE_PUT_PIXEL, x, y, pixel)
#define SYSCALL_SLEEP(ms) SYSCALL2(SYSCODE_SLEEP, ms, 0, 0)

typedef struct pargs_struct
{
    char** argv;
    int argc;
}pargs_t;
 
extern void exit(int code);
extern int main(int argc, char** argv);
 
void _start()
{
    pargs_t* args;
    SYSCALL_GET_ARGS(args);
    _init_signal();
    int ex = main(args->argc, args->argv);
    exit(ex);
}