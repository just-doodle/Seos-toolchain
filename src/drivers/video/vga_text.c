#include "vga_text.h"

static vga_text_t textmode;

static uint16_t* text_fb = (uint16_t*)VGA_TEXT_FB;

void init_text()
{
    textmode.height = 25;
    textmode.width = 80;

    textmode.x = 0;
    textmode.y = 0;

    textmode.fg = 0x0F;
    textmode.bg = 0x00;
}

void move_cursor()
{
    uint16_t pos = (textmode.y * 80 + textmode.x);
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void scroll()
{
    uint8_t attr = (textmode.bg << 4) | (textmode.fg & 0x0F);
    uint8_t space = 0x20 | (attr << 8);

    if(textmode.y >= textmode.height)
    {
        int i;
        for(i = 0*80; i < 24*80; i++)
        {
            text_fb[i] = text_fb[i+80];
        }

        for ( i = 24*80; i < 25*80; i++)
        {
            text_fb[i] = space;
        }

        textmode.y = 24;
    }
}

void text_putc(char c)
{
    uint8_t attr = (textmode.bg << 4) | (textmode.fg & 0x0F);
    uint16_t* location;

    if((c == 0x08) & (textmode.x > 0))
    {
        textmode.x--;
    }
    else if(c == '\t')
    {
        textmode.x = (textmode.x + 8) & ~(8 - 1);
    }
    else if(c == '\r')
    {
        textmode.x = 0;
    }
    else if(c == '\n')
    {
        textmode.x = 0;
        textmode.y++;
    }
    else if (c >= ' ')
    {
        location = text_fb + (textmode.y * textmode.width + textmode.x);
        *location = c | (attr << 8);
        textmode.x++;
    }

    if (textmode.x >= 80)
    {
        textmode.x = 0;
        textmode.y++;
    }

    scroll();
    move_cursor();
}

void text_clear()
{
    uint8_t attr = (textmode.bg << 4) | (textmode.fg & 0x0F);
    uint8_t space = 0x20 | (attr << 8);
    for(int i = 0*80; i < 25*80; i++)
    {
        text_fb[i] = space;
    }
}

void text_chcolor(uint8_t fg, uint8_t bg)
{
    textmode.bg = bg;
    textmode.fg = fg;
}

void text_puts(char* str)
{
    while(*str)
    {
        text_putc(*str);
        str++;
    }
}