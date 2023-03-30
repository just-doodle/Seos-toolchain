#include "debug.h"

#include "sym.h"

void kernel_panic(const char* message)
{
    text_chcolor(VGA_LIGHT_RED, VGA_BLACK);
    printf("kernel panic: - %s\n", message);
    while(1);
}

void stack_trace(uint32_t maxframes)
{
    stackframe_t* stk;
    asm("movl %%ebp,%0" : "=r"(stk) ::);
    printf("stack trace:\n");
    for(uint32_t frame = 0; stk && frame < maxframes; frame++)
    {
        printf("\t[0x%06x]: 0x%06x\n", ((uint32_t)stk), stk->eip);
        stk = stk->ebp;
    }
}

void backtrace()
{
    stackframe_t* stk;
    asm("movl %%ebp,%0" : "=r"(stk) ::);
    symoff_t symbol;
    printf("stack trace:\n");
    for(uint32_t frame = 0; stk && frame < 100; frame++)
    {
        symbol = get_symbol(stk->eip);
        printf("\t[0x%06x]: 0x%06x {%s+", ((uint32_t)stk), stk->eip, symbol.name);
        printf("0x%x}\n", symbol.offset);
        stk = stk->ebp;
    }
}