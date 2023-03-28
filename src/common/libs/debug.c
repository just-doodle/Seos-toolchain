#include "debug.h"
#include "elf.h"

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
    //! Not Implemented
    // Will be implemented once i implment malloc
}