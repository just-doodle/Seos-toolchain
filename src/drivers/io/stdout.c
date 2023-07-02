#include "stdout.h"
#include "keyboard.h"
#include "termios.h"
#include "charbuffer.h"
#include "se_term.h"

FILE* stdout_f;
FILE* stdin_f;
FILE* stderr_f;

uint32_t stdin_getfsize(FILE* f);

charbuffer_t* cbuf;

int stdio_ioctl(FILE* f, int request, void *data)
{
    switch (request)
    {
		case TIOCSWINSZ:
			return -1;
		case TIOCGWINSZ:
        {
            struct winsize w = get_winsize();
			memcpy(data, &w, sizeof(struct winsize));
			return 0;
        }break;
		default:
			return -1;
	}
	return -1;
}

void init_stdout()
{
    stdout_f = ZALLOC_TYPES(FILE);
    stdout_f->flags = FS_CHARDEVICE;
    stdout_f->mask = _IFCHR;
    stdout_f->write = stdout_write;
    stdout_f->open = stdout_open;
    stdout_f->close = stdout_close;
    stdout_f->ioctl = stdio_ioctl;
    strcpy(stdout_f->name, "stdout");

    stdin_f = ZALLOC_TYPES(FILE);
    stdin_f->flags = FS_CHARDEVICE;
    stdin_f->mask = _IFCHR;
    stdin_f->read = stdin_read;
    stdin_f->open = stdin_open;
    stdin_f->close = stdin_close;
    stdin_f->get_filesize = stdin_getfsize;
    stdin_f->ioctl = stdio_ioctl;
    strcpy(stdin_f->name, "stdin");

    stderr_f = ZALLOC_TYPES(FILE);
    stderr_f->flags = FS_CHARDEVICE;
    stderr_f->mask = _IFCHR;
    stderr_f->write = stderr_write;
    stderr_f->open = stderr_open;
    stderr_f->close = stderr_close;
    stderr_f->ioctl = stdio_ioctl;
    strcpy(stderr_f->name, "stderr");

    cbuf = create_charbuffer(512);

    devfs_add(stdout_f);
    devfs_add(stdin_f);
    devfs_add(stderr_f);
}

uint32_t stdout_write(FILE* f, uint32_t off, size_t sz, char* buffer)
{
    //serialprintf("%s", buffer);
    if(/*memcmp((uint8_t*)f, (uint8_t*)stdout_f, sizeof(FILE)) == 0*/1)
    {
        printf("%s", strndup(buffer, sz));
        memset(buffer, 0, sz);
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

void stdin_buf_clear()
{
    charbuffer_clean(cbuf);
}

void stdin_keybuff(uint8_t scancode)
{
    charbuffer_push(cbuf, kcodeTochar(scancode));
    //serialprintf("[STDIN] Got char %c in 0x%06x:0x%06x\n", cbuf->buffer[cbuf->rw_ptr-1], cbuf->rw_ptr-1, cbuf->rw_ptr);
}

uint32_t stdin_read(FILE* f, uint32_t off, uint32_t sz, char* buf)
{
    serialprintf("[STDIN] Waiting for char in off: 0x%06x+0x%06x\n", off, sz);
    ASM_FUNC("sti");
    charbuffer_waitchar(cbuf, '\n');
    charbuffer_dump(cbuf, buf);
    serialprintf("%s'C'\n", buf);
    return sz;
}

uint32_t stdin_getfsize(FILE* f)
{
    return cbuf->size;
}

void stdin_open(FILE* f, uint32_t flags)
{
    stdin_buf_clear();
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
        memset(buffer, 0, sz);
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