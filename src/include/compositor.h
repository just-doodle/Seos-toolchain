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

typedef uint32_t wid_t;

#define SEGUI_PIXELFORMAT_ARGB 0x01

typedef struct window_struct
{
    wid_t wid;

    char title[64];

    uint32_t x;
    uint32_t y;

    uint32_t width;
    uint32_t height;

    uint32_t bpp;
    uint32_t pitch;
    uint32_t fb_size;

    uint32_t bg;

    int isMinimized;
    int idx;

    uint32_t pixelformat; // Not used
    
    rect_region_t region;
    rect_region_t bar;

    uint32_t* userspace_fb_ptr;

    listnode_t* self;

    pid_t process;
}window_t;

typedef struct window_info_struct
{
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    char* title;
    uint32_t* fb_ptr; // To be used with window_display
    uint32_t x;
    uint32_t y;
    uint32_t pixelformat; // Not used
}window_info_t;

void init_compositor();

window_t* create_window(char* title, uint32_t width, uint32_t height, uint32_t x, uint32_t y);
window_t* create_window_from_info(window_info_t* info);

void window_move(window_t* w, uint32_t x, uint32_t y);
void window_focus(window_t* window);
void window_putpixel(window_t* win, uint32_t x, uint32_t y, uint32_t color);

void window_display(window_t* w, uint32_t* fb);
void window_swapBuffer(window_t* w, uint32_t* fb);

window_t* get_focused_window();
window_t* get_window(uint32_t x, uint32_t y);

void update_statusbar(char* msg, int s_col, int s_row, uint32_t color);

void window_change_title(window_t* w, char* title);
int compositor_on_mouse_move(uint32_t x, uint32_t y, int left, int right, int middle);

void update_video();

void window_drawall();

void window_close_by_pid(pid_t pid);

void maximize(window_t* w);
void minimize(window_t* w);

void compositor_background_fill();
void window_drawall();

#endif /*__COMPOSITOR_H__*/