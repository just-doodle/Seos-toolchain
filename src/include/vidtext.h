#ifndef __VIDTEXT_H__
#define __VIDTEXT_H__

#include "system.h"
#include "font.h"

#include "draw.h"
#include "kheap.h"
#include "printf.h"
#include "pit.h"
#include "bitmap.h"
#include "ifb.h"
#include "targa.h"

typedef struct vidtext
{
    uint32_t height;
    uint32_t width;

    uint32_t length;
    char *buffer;
    uint32_t pos;
    uint32_t pos_row;
    uint32_t pos_col;

    uint32_t col;
    uint32_t row;

    canvas_t canvas;

    uint32_t font_color;
    uint32_t background_color;

    bitmap_t *background_image;
} vidtext_t;

void init_vidtext();

void vidtext_set_font_color(uint32_t color);
void vidtext_set_background_color(uint32_t color);

void vidtext_putchar(char c);
void vidtext_clear();
void vidtext_printf(char *fmt, ...);
void vidtext_vprintf(char *fmt, va_list arg);
void vidtext_set_background_image(char *filename);
void vidtext_set_background_image_tga(char *filename);

void vidtext_update();

#endif /*__VIDTEXT_H__*/