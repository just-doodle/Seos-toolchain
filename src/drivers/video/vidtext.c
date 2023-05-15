#include "vidtext.h"
#include "compositor.h"
#include "targa.h"
#include "timer.h"

void vidtext_refresh();

vidtext_t *vidtext;
int vidtext_isInit = 0;

window_t* c_window = NULL;

rect_region_t vid_rect;
canvas_t vid_can;

uint32_t bg;
uint32_t fg;

void init_vidtext(uint32_t background_color)
{
    c_window = create_window("shell", 8*80, 8*25, 40, 40);

    vidtext = ZALLOC_TYPES(vidtext_t);

    vidtext->width = c_window->width;
    vidtext->height = c_window->height;

    vidtext->length = ((vidtext->width / FONT_WIDTH) * (vidtext->height / FONT_HEIGHT));
    vidtext->col = ((vidtext->width / FONT_WIDTH));
    vidtext->row = ((vidtext->height / FONT_HEIGHT));
    vidtext->background_image = NULL;

    vidtext->buffer = zalloc(vidtext->length);
    vidtext->canvas = canvas_create(vidtext->width, vidtext->height, c_window->region.region);

    vidtext->background_color = bg;
    vidtext->font_color = fg;

    vid_rect.r = rect_create(0, 0, c_window->width, c_window->height);
    vid_rect.region = zalloc(c_window->fb_size);

    vid_can = canvas_create(c_window->width, c_window->height, vid_rect.region);

    vidtext_isInit = true;

    register_wakeup_callback(vidtext_refresh, 60.0/get_frequency());
}

void vidtext_putchar(char c)
{
    if(vidtext_isInit == true)
    {
        if(c == 0)
            return;

        if(vidtext->pos_col == vidtext->col)
        {
            vidtext->buffer[vidtext->pos++] = '\n';
            vidtext->pos_row++;
            vidtext->pos_col = 0;
        }
        if (vidtext->pos_row > vidtext->row)
        {
            vidtext_clear();
        }

        if(c != (char)0xC0)
        {
            if(c == '\b')
            {
                vidtext->pos--;
                vidtext->pos_col--;
            }
            else if(c == '\n')
            {
                vidtext->pos_row++;
                vidtext->buffer[vidtext->pos++] = c;
                vidtext->pos_col = 0;
            }
            else
            {
                vidtext->buffer[vidtext->pos++] = c;
                vidtext->pos_col++;
            }
        }
    }
}

void vidtext_debug_dump()
{
    serialprintf("vidtext cursor position: 0x%06x\n", vidtext->pos);
    serialprintf("vidtext text buffer size: 0x%06x\n", vidtext->length);
    serialprintf("vidtext text buffer full: %s\n", (vidtext->pos > vidtext->length ? "Yes" : "No"));
    xxd(vidtext->buffer, vidtext->pos);
    return;
}

void vidtext_set_font_color(uint32_t color)
{
    if(vidtext != NULL)
        vidtext->font_color = color;
    else
        fg = color;
}



void vidtext_set_background_color(uint32_t color)
{
    if(vidtext != NULL)
        vidtext->background_color = color;
    else
        bg = color;
}

void vidtext_update()
{
    memsetdw(vid_rect.region, vidtext->background_color, c_window->fb_size/32);

    set_font_color(vidtext->font_color);

    vidtext->buffer[vidtext->pos] = '_';

    draw_text(&vid_can, vidtext->buffer, 0, 0);

    vidtext->buffer[vidtext->pos] = '\0';
}

void vidtext_draw()
{
    draw_rect_pixels(&vidtext->canvas, &vid_rect);
}

uint32_t z = 0;
void vidtext_refresh()
{
    z++;
    vidtext_draw();
    if(z == 1)
    {
        vidtext_update();
        z = 0;
    }
}

void vidtext_vprintf(char* fmt, va_list arg)
{
    vsprintf(NULL, vidtext_putchar, fmt, arg);
}

void vidtext_printf(char* fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);
    vidtext_vprintf(fmt, arg);
    va_end(arg);
}

void vidtext_clear()
{
    if(vidtext_isInit == true)
    {
        memset(vidtext->buffer, 0, vidtext->pos);
        vidtext->pos = 0;
        vidtext->pos_row = 0;
        vidtext->pos_col = 0;
        vidtext_update();
    }
}

void vidtext_set_background_image(char* filename)
{
    if(vidtext_isInit)
    {
        if (strlen(filename) == 0)
        {
            vidtext->background_image = NULL;
            return;
        }
        printf("loading bitmap image from: %s...\n", filename);
        vidtext->background_image = bitmap_create(filename);
    }
}