#define SYSCALL(ret, a, b, c, d) asm volatile("int $0x80": "=a"(ret) : "a"(a), "b"(b), "c"(c), "d"(d))
#define SYSCALL2(a, b, c, d) asm volatile("int $0x80": : "a"(a), "b"(b), "c"(c), "d"(d))

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

void puts(char* str)
{
    while(*str)
    {
        SYSCALL_PUTCHAR(*str);
        str++;
    }
}

char* buffer = 0;
int idx;

void clear_buffer()
{
    for(int j = 0; j < 255; j++)
        buffer[j] = 0;
    idx = 0;
}

void input_handler(char c, int isCTRL, int isALT, unsigned char scancode)
{
    SYSCALL_PUTCHAR(c);
    
    if(scancode == 0x01)
    {
        SYSCALL_CLEAR;
        puts("ECHO: ");
        return;
    }

    if(scancode == 0x3B)
    {
        SYSCALL_EXIT(0);
    }

    if(c == '\n')
    {
        buffer[idx++] = '\0';
        for(int a = 0; a < idx; a++)
            SYSCALL_PUTCHAR(buffer[a]);
        clear_buffer();
        puts("\nECHO: ");
        return;
    }

    buffer[idx++] = c;
}

void _start()
{
    SYSCALL_MALLOC(buffer, 256);
    char* hello = "Hello this is a program running on SectorOS-RW4 OS. This program echoes back anything you write\n";
    puts(hello);
    puts("ECHO: ");
    SYSCALL_ATTACH(input_handler);
    while(1);
}