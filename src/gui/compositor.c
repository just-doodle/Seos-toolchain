#include "compositor.h"
#include "ifb.h"
#include "targa.h"
#include "rtc.h"
#include "bitmap.h"
#include "logdisk.h"
#include "timer.h"
#include "mouse.h"

uint32_t background_color = rgb(254, 128, 20);

#define __COMPOSITOR_LOW_END__ 1

ifb_video_info_t* mode = NULL;

list_t* window_list;
window_t* focused_window = NULL;

canvas_t screen_canvas;

targa_t* wallpaper_t;
bitmap_t* wallpaper_b;

int wallpaper_type = 0;
int notify_timer = 0;

rect_region_t wallpaper;

rect_region_t statusbar;
rect_region_t notify;

list_t* notify_messages = NULL;

wid_t curr_wid = 0;

void fill(uint32_t color)
{
    register int i = 0;
    register uint32_t* fb = (void*)screen_canvas.framebuffer;
    register uint32_t fb_size = (mode->pitch) * mode->height;

    for(i = 0; i < fb_size; i++)
        fb[i] = color;
}

char time[128];

void show_time()
{
    timetochar(time, 0);
    update_statusbar(time, 0, 0, 0xFFFFFFFF);
}

window_t* create_window(char* title, uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    window_t* w = ZALLOC_TYPES(window_t);
    memcpy(w->title, title, 64);

    w->wid = curr_wid++;

    w->width = width;
    w->height = height;
    w->x = x;
    w->y = y-16;
    w->bpp = 32;
    w->pitch = w->width * w->bpp;
    w->fb_size = w->pitch * w->height;

    w->userspace_fb_ptr = NULL;

    w->self = list_insert_front(window_list, w);

    w->idx = list_contain(window_list, w);

    w->bar.r = rect_create(w->x, w->y, width, 16);
    w->bar.region = zalloc((width * w->bpp) * 16);

    focused_window = w;

    if(current_process != NULL)
        w->process = current_process->pid;
    else
        w->process  = __UINT32_MAX__;

    w->region.r.height = w->height;
    w->region.r.width = w->width;
    w->region.r.x = x;
    w->region.r.y = y;

    w->region.region = zalloc(w->fb_size);

    register uint32_t color = rand();
    memsetdw(w->region.region, color, (w->fb_size/32));

    serialprintf("Window %s: buffer: 0x%06x, size: %dB, end: 0x%06x\n", w->title, w->region.region, w->fb_size, (w->region.region + w->fb_size));

    set_fill_color(rgb(135, 6, 32));
    canvas_t c = canvas_create(w->bar.r.width, w->bar.r.height, w->bar.region);
    draw_rect(&c, 0, 0, c.width, c.height);
    draw_text(&c, w->title, 1, 0);

    return w;
}

void window_draw(window_t* win)
{
    if(win->region.region != NULL)
    {
        draw_rect_pixels(&screen_canvas, &(win->region));
    }
    if(win->bar.region != NULL)
    {
        canvas_t wbar = canvas_create(win->bar.r.width, win->bar.r.height, win->bar.region);
        round_corner_effect(&wbar);
        draw_rect_pixels(&screen_canvas, &win->bar);
    }
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
    //show_time();
    compositor_background_fill();
    register listnode_t* l = NULL;
    for(l = window_list->head; l != NULL; l = l->next)
    {
        if(l->val == focused_window)
            continue;
        if(((window_t*)l->val)->isMinimized == true)
            continue;
        window_draw(l->val);
    }
    window_draw(focused_window);
}__attribute__((optimize("-fno-tree-loop-distribute-patterns")))

void compositor_background_fill()
{
    set_fill_color(background_color);
    draw_rect(&screen_canvas, 0, 0, screen_canvas.width, screen_canvas.height);
    if(wallpaper_type != 0)
        draw_rect_pixels(&screen_canvas, &wallpaper);
}


int q = 0;


void win_timer()
{
    t++;

#if __COMPOSITOR_LOW_END__
    q++;

    if(q == 100)
    {
        show_time();
        q = 0;
    }
#endif


    if(t == 40)
    {
        window_drawall();
#if __COMPOSITOR_LOW_END__
        if(notify_timer != 0)
        {
            draw_rect_pixels(&screen_canvas, &notify);
            notify_timer--;
            if(notify_timer == 0)
            {
                set_fill_color(rgb(200, 109, 173));
                canvas_t ct = canvas_create(notify.r.width, 100, notify.region);
                draw_rect(&ct, 0, 0, ct.width, ct.height);
                compositor_background_fill();
                if((notify_messages != NULL) && list_size(notify_messages) != 0)
                {
                    listnode_t* ln = list_pop(notify_messages);
                    compositor_message_show(ln->val);
                    free(ln->val);
                }
            }
        }
#endif
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
    ASM_FUNC("cli");
    compositor_background_fill();
    ifb_refresh();

    win->x = x;
    win->y = y;

    if(x + win->width > screen_canvas.width)
        win->x = screen_canvas.width - win->width;

    if(y + win->height > screen_canvas.height)
        win->y = screen_canvas.height - win->height;

    win->region.r = rect_create(win->x, win->y+16, win->width, win->height);
    win->bar.r = rect_create(win->x, win->y, win->width, 16);

    window_drawall();
    ASM_FUNC("sti");
}

void window_focus(window_t* w)
{
    // serialprintf("[COMPOSITOR] Focused window \"%s\"\n", w->title);
    if(w->isMinimized == false)
        focused_window = w;
    else
        maximize(w);

    if(w->process != __UINT32_MAX__)
        if(current_process && (w->process == current_process->pid))
            return;
        else
            change_process(w->process);
}

void compositor_message_show(char* msg)
{
    if(notify_timer != 0)
    {
        if(notify_messages != NULL)
        {
            char* bu = strdup(msg);
            list_push(notify_messages, bu);
            return;
        }
    }
    set_fill_color(rgb(200, 109, 173));
    canvas_t ct = canvas_create(notify.r.width, 100, notify.region);
    draw_rect(&ct, 0, 0, ct.width, ct.height);
    draw_text(&ct, msg, 0, 0);
    notify_timer = 70;
}

int compositor_onModeset(ifb_video_info_t* vinfo)
{
    screen_canvas = canvas_create(vinfo->width, vinfo->height, ifb_getIFB());
    set_fill_color(rgb(254, 128, 20));
    draw_rect(&screen_canvas, 0, 0, vinfo->width, vinfo->height);

    statusbar.r = rect_create(0, 0, vinfo->width, 16);
    statusbar.region = realloc(statusbar.region, (vinfo->width * 32)*16);

    wallpaper.r = rect_create(0, 16, vinfo->width, vinfo->height-16);
    wallpaper.region = realloc(wallpaper.region, (vinfo->width*vinfo->bpp)*vinfo->height);

    notify.r = rect_create(vinfo->width-(33*8), statusbar.r.height+2, (32*8), 100);
    notify.region = realloc(notify.region, (notify.r.width*32)*notify.r.height);

    mode = vinfo;

    ldprintf("Compositor", LOG_DEBUG, "Changing resolution of the compositor to %dx%dx%d", vinfo->width, vinfo->height, vinfo->bpp);

    compositor_background_fill();
    window_drawall();
    ifb_refresh();

    return 0;
}

void init_compositor()
{
    mode = video_get_modeinfo();
    if(validate(mode) != 1)
        return;

    window_list = list_create();
    notify_messages = list_create();
    screen_canvas = canvas_create(mode->width, mode->height, ifb_getIFB());

    set_fill_color(rgb(254, 128, 20));
    draw_rect(&screen_canvas, 0, 0, mode->width, mode->height);

    statusbar.r = rect_create(0, 0, mode->width, 16);
    statusbar.region = zalloc((mode->width * 32)*16);

    wallpaper.r = rect_create(0, 16, mode->width, mode->height-16);
    wallpaper.region = zalloc((mode->pitch)*mode->height);

    notify.r = rect_create(mode->width-(33*8), statusbar.r.height+2, (32*8), 100);
    notify.region = zalloc((notify.r.width*32)*notify.r.height);

    update_statusbar("Hello, This is a test build on SEGUI.", 0, 0, 0xFFaaffff);

    register_modeset_handler(compositor_onModeset);

    register_wakeup_callback(win_timer, 60/(get_frequency() == 0 ? 1000 : get_frequency()));
    ldprintf("Compositor", LOG_INFO, "Compositor successfully initialized");
}

window_t* get_focused_window()
{
    return focused_window;
}

void window_close(window_t* w)
{
    list_remove_node(window_list, w->self);
    free(w->region.region);
    free(w->bar.region);
    free(w);
    ASM_FUNC("cli");
    window_drawall();
    ASM_FUNC("sti");
}

void window_close_norefresh(window_t* w)
{
    printf("[SECOMP] Closing window \"%s\"\n", w->title);
    focused_window = (w->self->next->val == NULL ? (w->self->prev->val == NULL ? NULL : w->self->prev->val) : w->self->next->val);
    list_remove_node(window_list, w->self);
    free(w->region.region);
    free(w->bar.region);
    free(w);
}

void window_close_by_pid(pid_t pid)
{
    for(register listnode_t* l = window_list->head; l != NULL; l = l->next)
    {
        register window_t *w = l->val;
        if(w->process == pid)
        {
            window_close_norefresh(w);
        }
    }
    ASM_FUNC("cli");
    compositor_background_fill();
    window_drawall();
    ASM_FUNC("sti");
}

void update_statusbar(char* msg, int s_col, int s_row, uint32_t color)
{
    canvas_t st = canvas_create(mode->width, 16, statusbar.region);
    set_fill_color(rgb(135, 6, 41));
    draw_rect(&st, 0, 0, st.width, st.height);
    set_font_color(color);
    draw_text(&st, msg, s_row, s_col);
}

void window_display(window_t* w, uint32_t* fb)
{
    if(fb != NULL)
        memcpy(w->region.region, fb, w->fb_size);
}

void window_swapBuffer(window_t* w, uint32_t* fb)
{
    w->userspace_fb_ptr = fb;
}

window_t* create_window_from_info(window_info_t* info)
{
    window_t* w = ZALLOC_TYPES(window_t);
    memcpy(w->title, info->title, 64);

    w->wid = curr_wid++;

    w->width = info->width;
    w->height = info->height;
    w->x = info->x;
    w->y = info->y-16;
    w->bpp = info->bpp;
    w->pitch = w->width * w->bpp;
    w->fb_size = w->pitch * w->height;

    w->userspace_fb_ptr = info->fb_ptr;

    w->self = list_insert_front(window_list, w);

    w->idx = list_contain(window_list, w);

    w->bar.r = rect_create(w->x, w->y, info->width, 16);
    w->bar.region = zalloc((info->width * w->bpp) * 16);

    focused_window = w;

    if(current_process != NULL)
        w->process = current_process->pid;
    else
        w->process  = __UINT32_MAX__;

    w->region.r.height = w->height;
    w->region.r.width = w->width;
    w->region.r.x = info->x;
    w->region.r.y = info->y;

    w->region.region = zalloc(w->fb_size);

    register uint32_t color = rand();
    memsetdw(w->region.region, color, (w->fb_size/32));

    serialprintf("Window %s: buffer: 0x%06x, size: %dB, end: 0x%06x\n", w->title, w->region.region, w->fb_size, (w->region.region + w->fb_size));

    set_fill_color(rgb(135, 6, 32));
    canvas_t c = canvas_create(w->bar.r.width, w->bar.r.height, w->bar.region);
    draw_rect(&c, 0, 0, c.width, c.height);
    draw_text(&c, w->title, 1, 0);

    return w;
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

int isMoving = 0;
window_t* moving_window = NULL;

int compositor_on_mouse_move(uint32_t x, uint32_t y, int left, int right, int middle)
{
    if(left)
    {
        if(!isMoving)
        {
            window_t *w = get_window(x, y);
            if(!w)
                return 0;
            ldprintf("Mouse", LOG_DEBUG, "Moving window %s to (%d, %d)", w->title, x, y);
            moving_window = w;
            isMoving = 1;
        }
    }
    else
    {
        if(isMoving)
        {
            window_move(moving_window, x, y);
            moving_window = NULL;
            isMoving = 0;
        }
    }
}

void compositor_load_wallpaper(char* file, int type)
{
    if(type == 1) //targa
    {
        wallpaper_t = targa_parse(file);
        targa_framebuffer(wallpaper_t, wallpaper.region);
        wallpaper_type = 1;
    }

    if(type == 2) //bitmap
    {
        wallpaper_b = bitmap_create(file);
        if(wallpaper_b->height != screen_canvas.height || wallpaper_b->width != screen_canvas.width)
            return;
        if(wallpaper_b->bpp == 24)
            bitmap_to_framebuffer(wallpaper_b, wallpaper.region);
        else
            bitmap_to_framebuffer2(wallpaper_b, wallpaper.region);

        wallpaper_type = 2;
    }
    compositor_background_fill();
}

void minimize_focused()
{
    focused_window->isMinimized = true;
    focused_window = (focused_window->self->next->val == NULL ? (focused_window->self->prev->val == NULL ? NULL : focused_window->self->prev->val) : focused_window->self->next->val);
    ASM_FUNC("cli");
    compositor_background_fill();
    window_drawall();
    ASM_FUNC("sti");
}

void maximize(window_t* w)
{
    if(w->isMinimized == true)
    {
        w->isMinimized = false;
        focused_window = w;
        ASM_FUNC("cli");
        compositor_background_fill();
        window_drawall();
        ASM_FUNC("sti");
    }
}

void minimize(window_t* w)
{
    if(w->isMinimized == false)
    {
        w->isMinimized = true;
        if(w == focused_window)
        {
            focused_window = (focused_window->self->next->val == NULL ? (focused_window->self->prev->val == NULL ? NULL : focused_window->self->prev->val) : focused_window->self->next->val);
        }
        ASM_FUNC("cli");
        compositor_background_fill();
        window_drawall();
        ASM_FUNC("sti");
    }
}

int win_nn = 0;

void compositor_focus_next()
{
    win_nn++;
    if(list_size(window_list) < win_nn)
        win_nn = list_size(window_list);

    if(list_get_node_by_index(window_list, win_nn) != NULL)
        window_focus(list_get_node_by_index(window_list, win_nn)->val);
}

void compositor_focus_previous()
{
    win_nn--;
    if(win_nn < 0)
        win_nn = 0;

    if(list_get_node_by_index(window_list, win_nn) != NULL)
        window_focus(list_get_node_by_index(window_list, win_nn)->val);
}