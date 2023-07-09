#include "font.h"

uint32_t font_color = VESA_COLOR_BLACK + 1;

uint32_t get_font_color()
{
    return font_color;
}

void set_font_color(uint32_t color)
{
    font_color = color | 0xff000000;
}

void draw_text(canvas_t * canvas, char * text, int start_x, int start_y)
{
    uint32_t i, x, y, col, row, stop;
    uint8_t *fnt, chr;

    fnt = get_font();
    int len = strlen(text);

    col = start_y;
    row = start_x;
    stop = 0;

    for(i = 0; i < len; i++)
    {
        switch(text[i])
        {
            case '\n':
                row++;
                col = 0;
                chr = 0;
                break;

            case '\r':
                chr = 0;
                break;

            case '\t':
                chr = 0;
                col += 4 - col % 4;
                break;

            case '\0':
                stop = 1;
                chr = 0;
                break;

            default:
                col++;
                if(col > canvas->width/FONT_WIDTH)
                {
                    row++;
                    col = 0;
                    if(row >= canvas->height/FONT_HEIGHT)
                    {
                        row = 0;
                        col = 0;
                    }
                }
                chr = text[i];
                break;
        }

        if(stop)
        {
            break;
        }

        if(chr != 0 && canvas->width != 0)
        {
            for(y = 0; y < FONT_HEIGHT; y++)
            {
                for(x = 0; x < FONT_WIDTH; x++)
                {
                    if(fnt[text[i] * FONT_HEIGHT + y] >> (7 - x) & 1)
                    {
                        canvas->framebuffer[((col - 1) * FONT_WIDTH) + x + (y + row * FONT_HEIGHT) * canvas->width] = font_color;
                    }
                }
            }
        }
    }
}