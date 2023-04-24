#ifndef __FONT_H__
#define __FONT_H__

#include "draw.h"
#include "system.h"
#include "printf.h"

#define FONT_HEIGHT 8
#define FONT_WIDTH  8

typedef uint8_t* font_t;

font_t get_font();

uint32_t get_Font_color();
void set_font_color(uint32_t color);

void draw_text(canvas_t * canvas, char * text, int start_x, int start_y);

#endif /*__FONT_H__*/