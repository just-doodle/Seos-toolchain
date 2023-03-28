#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "system.h"
#include "vga_text.h"
#include "printf.h"

typedef struct stackframe
{
    struct stackframe *ebp;
    uint32_t eip;
}stackframe_t;

void panic(char* message, char* file);
void kernel_panic(const char* message);
void stack_trace(uint32_t maxframes);
void backtrace();

#endif /*__DEBUG_H__*/