#include "stdout.h"
#include "keyboard.h"

FILE* stdout_f;
FILE* stdin_f;
FILE* stderr_f;

void init_stdout()
{
    stdout_f = ZALLOC_TYPES(FILE);
    stdout_f->flags = FS_CHARDEVICE;
    stdout_f->mask = _IFCHR;
    stdout_f->write = stdout_write;
    stdout_f->open = stdout_open;
    stdout_f->close = stdout_close;
    strcpy(stdout_f->name, "stdout");

    stdin_f = ZALLOC_TYPES(FILE);
    stdin_f->flags = FS_CHARDEVICE;
    stdin_f->mask = _IFCHR;
    stdin_f->read = stdin_read;
    stdin_f->open = stdin_open;
    stdin_f->close = stdin_close;
    strcpy(stdin_f->name, "stdin");

    stderr_f = ZALLOC_TYPES(FILE);
    stderr_f->flags = FS_CHARDEVICE;
    stderr_f->mask = _IFCHR;
    stderr_f->write = stderr_write;
    stderr_f->open = stderr_open;
    stderr_f->close = stderr_close;
    strcpy(stderr_f->name, "stderr");

    devfs_add(stdout_f);
    devfs_add(stdin_f);
    devfs_add(stderr_f);
}

uint32_t stdout_write(FILE* f, uint32_t off, size_t sz, char* buffer)
{
    serialprintf("[%s] %s", f, buffer);
    if(memcmp(f, stdout_f, sizeof(FILE)) == 0)
    {
        printf("%s", buffer);
        return sz;
    }
    return -1;
}

void stdout_open(FILE* f, uint32_t flags)
{
    if(f == stdout_f)
    {
        if(flags & OPEN_TRUNC)
        {
            text_clear();
        }
    }
    return;
}

void stdout_close(FILE* f)
{
    return;
}


char stdin_buf[1028];
size_t stdin_idx = 0;

void stdin_buf_clear()
{
    for(int i = 0; i < 1028; i++)
    {
        stdin_buf[i] = '\0';
    }
    stdin_idx = 0;
}

void stdin_keybuff(uint8_t scancode)
{
    if(stdin_idx >= 1028)
    {
        stdin_buf_clear();
    }
    char c = kcodeTochar(scancode);
    if(c == '\0')
        return 0;
    stdin_buf[stdin_idx++] = c;
    serialprintf("[STDIN] Got char %c in 0x%06x:0x%06x\n", stdin_buf[stdin_idx-1], stdin_idx-1, stdin_idx);
}

uint32_t stdin_read(FILE* f, uint32_t off, uint32_t sz, char* buf)
{
    if(f == stdin_f)
    {
        serialprintf("[STDIN] Waiting for char in off: 0x%06x+0x%06x\n", off, sz);
        stdin_buf_clear();
        asm("sti");
        while(/*stdin_buf[off+sz] == '\0' &&*/ stdin_buf[stdin_idx-1] != '\n');
        memcpy(buf, stdin_buf+off, sz);
        //stdin_buf_clear();
        return sz;
    }
    return -1;
}

void stdin_open(FILE* f, uint32_t flags)
{
    return;
}

void stdin_close(FILE* f)
{
    return;
}

uint32_t stderr_write(FILE* f, uint32_t off, size_t sz, char* buffer)
{
    serialprintf("[STDERR] %s", buffer);
    if(f == stderr_f)
    {
        printf("%s", buffer);
        return sz;
    }
    return -1;
}

void stderr_open(FILE* f, uint32_t flags)
{
    return;
}

void stderr_close(FILE* f)
{
    return;
}