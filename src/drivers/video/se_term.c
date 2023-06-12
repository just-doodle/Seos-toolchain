#include "se_term.h"
#include "timer.h"
#include "unifont.h"
#include "termios.h"

window_t* win;

struct framebuffer_t fb;
term_t* term = NULL;

#define TERM_WIDTH 640
#define TERM_HEIGHT 400

void init_seterm()
{
    struct font_t font = {
        unifont,
        UNIFONT_WIDTH,
        UNIFONT_HEIGHT,
        DEFAULT_FONT_SPACING,
        DEFAULT_FONT_SCALE_X,
        DEFAULT_FONT_SCALE_Y
    };

    struct style_t style = {
        DEFAULT_ANSI_COLOURS,
        DEFAULT_ANSI_BRIGHT_COLOURS,
        DEFAULT_BACKGROUND,
        DEFAULT_FOREGROUND_BRIGHT,
        DEFAULT_BACKGROUND_BRIGHT,
        DEFAULT_FOREGROUND_BRIGHT,
        DEFAULT_MARGIN,
        DEFAULT_MARGIN_GRADIENT
    };

    struct background_t back = {
        NULL, /* You can set this to NULL to disable background */
        IMAGE_TILED,
        DEFAULT_BACKDROP
    };

    win = create_window("terminal", TERM_WIDTH, TERM_HEIGHT, 45, 45);

    fb.height = win->height;
    fb.width = win->width;
    fb.pitch = 4*win->width;
    fb.address = (uint32_t)win->region.region;

    serialprintf("[TERM] H: %d W: %d P: %d ADDR: 0x%x\n", fb.height, fb.width, fb.pitch, fb.address);

    term = term_init(fb, font, style, back);

    term_write(term, "Hello World!\n", 14);

    term_clear();
}

void term_putchar(char c)
{
    if(term == NULL)
        return;
    term_write(term, &c, 1);
}

void term_clear()
{
    if(term == NULL)
        return;
    term->clear(term, true);
    term->full_refresh(term);
}

void term_printf(char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsprintf(NULL, term_putchar, fmt, args);
    va_end(args);
}

void *term_alloc(size_t size) 
{
    return kmalloc(size);
}

void *term_realloc(void *oldptr, size_t size)
{
    return krealloc(oldptr, size);
}

// size is the same value that was passed to term_alloc()
void term_free(void *ptr, size_t size)
{
    kfree(ptr);
}

void term_freensz(void *ptr)
{
    kfree(ptr);
}

void *term_memcpy(void *dest, const void *src, size_t size)
{
    return memcpy(dest, src, size);
}

void *term_memset(void *dest, int val, size_t count)
{
    return memset(dest, val, count);
}

struct winsize get_winsize()
{
    struct winsize w;
    w.ws_col = term->cols;
    w.ws_row = term->rows;
    w.ws_xpixel = fb.width;
    w.ws_ypixel = fb.height;

    return w;
}