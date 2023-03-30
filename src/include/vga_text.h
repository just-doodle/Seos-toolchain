#ifndef __VGA_TEXT__
#define __VGA_TEXT__

#include "system.h"
#include "ports.h"

#define VGA_TEXT_FB (0xB8000 + KERNEL_BASE)


#define VGA_BLACK 0
#define VGA_BLUE 1
#define VGA_GREEN 2
#define VGA_CYAN 3
#define VGA_RED 4
#define VGA_MAGENTA 5
#define VGA_BROWN 6
#define VGA_LIGHT_GREY 7
#define VGA_DARK_GREY 8
#define VGA_LIGHT_BLUE 9
#define VGA_LIGHT_GREEN 10
#define VGA_LIGHT_CYAN 11
#define VGA_LIGHT_RED 12
#define VGA_LIGHT_MAGENTA 13
#define VGA_LIGHT_BROWN 14
#define VGA_WHITE 15

typedef struct vga_text
{
    uint32_t x;
    uint32_t y;

    uint32_t height;
    uint32_t width;

    uint8_t fg;
    uint8_t bg;
}vga_text_t;

void init_text();

void text_putc(char c);
void text_puts(char* str);

void text_chcolor(uint8_t fg, uint8_t bg);
void text_clear();

#endif /*__VGA_TEXT__*/