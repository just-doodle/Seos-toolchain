#ifndef __DRAW_H__
#define __DRAW_H__

#include "system.h"
#include "string.h"
#include "kheap.h"
#include "vesa.h"

typedef struct rect_struct
{
    int x;
    int y;
    int width;
    int height;
}rect_t;

typedef struct rect_region_struct
{
    rect_t r;
    uint32_t * region;
}rect_region_t;

typedef struct canvas_struct
{
    int width;
    int height;
    uint32_t * framebuffer;
}canvas_t;

#define GET_ALPHA(val) val & 0xFF000000
#define GET_RED(val) val & 0x00FF0000
#define GET_GREEN(val) val & 0x0000FF00
#define GET_BLUE(val) val & 0x000000FF
#define SET_ALPHA(color, alpha) ( ((color << 8) >> 8) | ((alpha << 24) & 0xff000000))

#define rgb(r, g, b) ((0x00ff0000 & (r << 16)) | (0x0000ff00 & (g << 8)) | (0x000000ff & b))
#define argb(a, r, g, b) (0xff000000 & (a << 24)) | (0x00ff0000 & (r << 16)) | (0x0000ff00 & (g << 8)) | (0x000000ff & b)

#define VESA_COLOR_BLACK rgb(0,0,0)
#define VESA_COLOR_WHITE rgb(255, 255, 255)
#define VESA_COLOR_RED   rgb(255, 0, 0)
#define VESA_COLOR_GREEN rgb(0, 255, 0)
#define VESA_COLOR_BLUE  rgb(0, 0, 255)
#define VESA_COLOR_MAGENTA  rgb(255, 0, 255)

#define BIOS_COLOR0 rgb(0, 0, 0)
#define BIOS_COLOR1 rgb(0, 0, 170)
#define BIOS_COLOR2 rgb(0, 170, 0)
#define BIOS_COLOR3 rgb(0, 170, 170)
#define BIOS_COLOR4 rgb(170, 0, 0)
#define BIOS_COLOR5 rgb(170, 0, 170)
#define BIOS_COLOR6 rgb(170, 85, 0)
#define BIOS_COLOR7 rgb(170, 170, 170)
#define BIOS_COLOR8 rgb(85, 85, 85)
#define BIOS_COLOR9 rgb(85, 85, 255)
#define BIOS_COLOR10 rgb(85, 255, 85)
#define BIOS_COLOR11 rgb(85, 255, 255)
#define BIOS_COLOR12 rgb(255, 85, 85)
#define BIOS_COLOR13 rgb(223, 193, 223)
#define BIOS_COLOR14 rgb(255, 255, 85)
#define BIOS_COLOR15 rgb(226, 226, 226)

#define BIOS_COLOR(x) BIOS_COLOR##x

extern uint32_t* screen;
extern int screen_width;
extern int screen_height;

int get_pixel_idx(canvas_t * canvas, int x, int y);

void set_pixel(canvas_t * canvas, uint32_t val, int x, int y);

void remove_sharp_edges(canvas_t * canvas, int start_x, int start_y, int direction, int num_pixels, uint32_t end_alpha, uint32_t middle_alpha);

void round_corner_effect(canvas_t * canvas);

void set_fill_color(uint32_t color);

void draw_rect(canvas_t * t, int x, int y, int width, int height);

void draw_rect_pixels(canvas_t * canvas, rect_region_t * rect_region);
void draw_rect_clip_pixels(canvas_t * canvas, rect_region_t * rect_region, int rect_true_width);
void draw_rect_clip_pixels2(canvas_t * canvas, rect_region_t * rect_region, int rect_true_width, int rect_true_x, int rect_true_y);

void draw_line(canvas_t * canvas, int x1, int y1, int x2, int y2);
int is_line_overlap(int line1_x1, int line1_x2, int line2_x1, int line2_x2);

uint32_t bioscolor_to_vesa(uint32_t bios);

int is_rect_overlap(rect_t rect1, rect_t rect2);
rect_t find_rect_overlap(rect_t rect1, rect_t rect2);
int is_point_in_rect(int point_x, int point_y, rect_t * r);
rect_t rect_create(int x, int y, int width, int height);

canvas_t canvas_create(int width, int height, uint32_t * framebuffer);

uint32_t blend_colors(uint32_t color1, uint32_t color2);
#endif /*__DRAW_H__*/