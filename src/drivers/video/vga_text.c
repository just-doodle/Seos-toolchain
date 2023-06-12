#include "vga_text.h"
#include "vidtext.h"

static vga_text_t textmode;

static uint16_t* text_fb = (uint16_t*)VGA_TEXT_FB;

void disable_blink()
{
    uint32_t a = inb(0x3DA);
    outb(0x3C0, 0x30);
    a = inb(0x3C1);
    a = a & 0xF7;
    outb(0x3C0, a);
}

void move_cursor()
{
    uint16_t pos = (textmode.y * 80 + textmode.x);
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void init_text()
{
    textmode.height = 25;
    textmode.width = 80;

    textmode.x = 0;
    textmode.y = 0;

    textmode.fg = VGA_WHITE;
    textmode.bg = VGA_LIGHT_BLUE;
    disable_blink();
    move_cursor();
}

void scroll()
{
    uint8_t attr = ((textmode.bg << 4) & 0xF0) | (textmode.fg & 0x0F);
    uint16_t space = 0x20 | (attr << 8);

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
#if 0
    uint8_t attr = ((textmode.bg << 4) & 0xF0) | (textmode.fg & 0x0F);
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
#else
    term_putchar(c);
#endif
}

void text_clear()
{
#if 0
    uint8_t attr = ((textmode.bg << 4) & 0xF0) | (textmode.fg & 0x0F);
    uint16_t space = 0x20 | (attr << 8);
    for(int i = 0*80; i < 25*80; i++)
    {
        text_fb[i] = space;
    }
    textmode.x = 0;
    textmode.y = 0;
    move_cursor();
#else
    term_clear();
#endif
}

void text_chcolor(uint8_t fg, uint8_t bg)
{
    textmode.bg = bg;
    textmode.fg = fg;
}

uint8_t text_getBG()
{
    return textmode.bg;
}

uint8_t text_getFG()
{
    return textmode.fg;
}

void text_puts(char* str)
{
    while(*str)
    {
        text_putc(*str);
        str++;
    }
}