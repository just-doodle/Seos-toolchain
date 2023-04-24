#ifndef __COMPOSITOR_H__
#define __COMPOSITOR_H__

#include "system.h"
#include "tree.h"
#include "list.h"
#include "kheap.h"
#include "draw.h"
#include "bitmap.h"
#include "keyboard.h"
#include "font.h"
#include "process.h"

typedef struct window_struct
{
    char title[64];

    uint32_t x;
    uint32_t y;

    uint32_t width;
    uint32_t height;

    uint32_t bpp;
    uint32_t pitch;
    uint32_t fb_size;

    uint32_t bg;

    int idx;
    
    rect_region_t region;
    rect_region_t bar;

    listnode_t* self;

    pid_t process;
}window_t;

window_t* create_window(char* title, uint32_t width, uint32_t height, uint32_t x, uint32_t y);
void window_move(window_t* w, uint32_t x, uint32_t y);
void window_focus(window_t* window);
void window_putpixel(window_t* win, uint32_t x, uint32_t y, uint32_t color);

window_t* get_focused_window();
window_t* get_window(uint32_t x, uint32_t y);

void update_statusbar(char* msg, int s_col, int s_row, uint32_t color);

void window_display(window_t* w, uint32_t* framebuffer);
void window_change_title(window_t* w, char* title);

void update_video();

#endif /*__COMPOSITOR_H__*/