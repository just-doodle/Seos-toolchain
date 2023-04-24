#include "compositor.h"
#include "ifb.h"
#include "targa.h"
#include "rtc.h"
#include "bitmap.h"

VBE_MODE_INFO_t* mode = NULL;

list_t* window_list;
window_t* focused_window = NULL;

canvas_t screen_canvas;

targa_t* wallpaper;

rect_region_t statusbar;

void fill(uint32_t color)
{
    register int i = 0;
    register uint32_t* fb = (void*)screen_canvas.framebuffer;
    register uint32_t fb_size = mode->pitch * mode->YResolution;

    for(i = 0; i < fb_size; i++)
        fb[i] = color;
}

char time[64];

void show_time()
{
    memset(time, 0, 64);
    rtc_char(time, 0);
    update_statusbar(time, 0, 0, 0xFFFFFFFF);
}

window_t* create_window(char* title, uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    window_t* nw = ZALLOC_TYPES(window_t);
    memcpy(nw->title, title, 64);

    nw->width = width;
    nw->height = height;
    nw->x = x;
    nw->y = y+16;
    nw->bpp = 32;
    nw->pitch = nw->width * nw->bpp;
    nw->fb_size = nw->pitch * nw->height;

    nw->self = list_insert_front(window_list, nw);

    nw->idx = list_contain(window_list, nw);

    nw->bar.r = rect_create(nw->x, nw->y, width, 16);
    nw->bar.region = zalloc((width * nw->bpp) * 16);

    focused_window = nw;

    if(current_process != NULL)
        nw->process = current_process->pid;
    else
        nw->process  = __UINT32_MAX__;

    nw->region.r.height = nw->height;
    nw->region.r.width = nw->width;
    nw->region.r.x = nw->x;
    nw->region.r.y = nw->y;

    nw->region.region = zalloc(nw->fb_size);

    serialprintf("Window %s: buffer: 0x%06x, size: %dB, end: 0x%06x\n", nw->title, nw->region.region, nw->fb_size, (nw->region.region + nw->fb_size));

    set_fill_color(rgb(135, 6, 32));
    canvas_t c = canvas_create(nw->bar.r.width, nw->bar.r.height, nw->bar.region);
    draw_rect(&c, 0, 0, c.width, c.height);
    draw_text(&c, nw->title, 1, 0);

    return nw;
}

void window_draw(window_t* win)
{
    if(win->region.region != NULL)
        draw_rect_pixels(&screen_canvas, &(win->region));
    if(win->bar.region != NULL)
        draw_rect_pixels(&screen_canvas, &win->bar);

    draw_rect_pixels(&screen_canvas, &statusbar);
}

void focus_draw()
{
    window_draw(focused_window);
}

void window_sort()
{
    list_remove_by_index(window_list, focused_window->idx);
    focused_window->self = list_insert_front(window_list, focused_window);
    focused_window->idx = list_contain(window_list, focused_window);
}

uint32_t t = 0;

void window_drawall()
{
    show_time();
    register listnode_t* l = NULL;
    for(l = window_list->head; l != NULL; l = l->next)
    {
        if(l->val == focused_window)
            continue;
        window_draw(l->val);
    }
    window_draw(focused_window);
}__attribute__((optimize("-fno-tree-loop-distribute-patterns")))

void win_timer()
{
    t++;
    if(t == 70)
    {
        vidtext_refresh();
        window_drawall();
        t = 0;
    }
}

void window_putpixel(window_t* win, uint32_t x, uint32_t y, uint32_t color)
{
    uint32_t idx = win->width * y + x;
    win->region.region[idx] = color & 0xFF000000;
}

void window_move(window_t* win, uint32_t x, uint32_t y)
{
    asm("cli");
    set_fill_color(rgb(254, 128, 20));
    draw_rect(&screen_canvas, 0, 0, mode->XResolution, mode->YResolution);
    ifb_refresh();

    win->x = x;
    win->y = y;

    win->region.r = rect_create(win->x, win->y+16, win->width, win->height);
    win->bar.r = rect_create(win->x, win->y, win->width, 16);

    window_drawall();
    ifb_refresh();
    asm("sti");
}

void window_focus(window_t* w)
{
    serialprintf("[COMPOSITOR] Focused window \"%s\"\n", w->title);
    focused_window = w;

    if(w->process != __UINT32_MAX__)
        change_process(w->process);
}

void compositor_message_show(char* msg)
{
    //TODO
}

void init_compositor()
{
    mode = vesa_get_current_mode();

    window_list = list_create();
    screen_canvas = canvas_create(mode->XResolution, mode->YResolution, ifb_getIFB());

    set_fill_color(rgb(254, 128, 20));
    draw_rect(&screen_canvas, 0, 0, mode->XResolution, mode->YResolution);

    statusbar.r = rect_create(0, 0, mode->XResolution, 16);
    statusbar.region = zalloc((mode->XResolution * 32)*16);

    update_statusbar("Hello, This is a test build on SECOMP.", 0, 0, 0xFFaaffff);

    pit_register(win_timer, 20);
}

window_t* get_focused_window()
{
    return focused_window;
}

void update_statusbar(char* msg, int s_col, int s_row, uint32_t color)
{
    canvas_t st = canvas_create(mode->XResolution, 16, statusbar.region);
    set_fill_color(rgb(135, 6, 41));
    draw_rect(&st, 0, 0, st.width, st.height);
    set_font_color(color);
    draw_text(&st, msg, s_row, s_col);
}

void window_display(window_t* w, uint32_t* framebuffer)
{
    memcpy(w->region.region, framebuffer, w->fb_size);
}

void window_change_title(window_t* w, char* title)
{
    memcpy(w->title, title, 64);
    set_fill_color(rgb(135, 6, 32));
    canvas_t c = canvas_create(w->bar.r.width, w->bar.r.height, w->bar.region);
    draw_rect(&c, 0, 0, c.width, c.height);
    draw_text(&c, w->title, 1, 0);
}

window_t* get_window(uint32_t x, uint32_t y)
{
    register listnode_t* l = NULL;
    for(l = window_list->head; l != NULL; l = l->next)
    {
        window_t* w = l->val;
        if(is_point_in_rect(x, y, &w->region.r) || is_point_in_rect(x, y, &w->bar.r))
        {
            return w;
        }
    }
    return NULL;
}