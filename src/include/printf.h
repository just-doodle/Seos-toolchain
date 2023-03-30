#ifndef __PRINTF_H__
#define __PRINTF_H__

#include "system.h"
#include "string.h"
#include "vga_text.h"

void vsprintf(char *buf, void (*putchar)(char), const char *fmt, va_list args);
void printf(const char *fmt, ...);
void serialprintf(const char* fmt, ...);

int is_fmt_letter(char c);

#endif /*__PRINTF_H__*/