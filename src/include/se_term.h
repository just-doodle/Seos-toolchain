#ifndef __VIDTEXT_H__
#define __VIDTEXT_H__

#define FLANTERM_FB_DISABLE_CANVAS

#include "system.h"
#include "font.h"
#include "compositor.h"

#include "draw.h"
#include "kheap.h"
#include "printf.h"
#include "ifb.h"
#include "limine_terminal/term.h"
#include <limine_terminal/image.h>
#include "font.h"

void init_term();
void term_putchar(char c);
void term_clear();
void term_printf(char* fmt, ...);

struct winsize get_winsize();

#endif /*__VIDTEXT_H__*/