#include "debug.h"


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

#define DUMP_COLS 16

void xxd(void* ptr, uint32_t size)
{
    uint32_t i;
    uint32_t j;

    for(i = 0; i < size + ((size % DUMP_COLS) ? (DUMP_COLS - size % DUMP_COLS) : 0); i++) 
    {
        if(i % DUMP_COLS == 0) 
        {
            serialprintf("0x%06x: ", i);
        }

        if(i < size) 
        {
            serialprintf("%02x ", 0xFF & ((char*)ptr)[i]);
        }
        else
        {
            serialprintf("   ");
        }

        if(i % DUMP_COLS == (DUMP_COLS - 1))
        {
            for(j = i - (DUMP_COLS - 1); j <= i; j++)
            {
                if(j >= size)
                {
                    serialprintf(" ");
                }
                else if(isprint(((char*)ptr)[j]))
                {
                    serialprintf("%c", (((char*)ptr)[j]) & 0xFF);
                }
                else
                {
                    serialprintf(".");
                }
            }
            serialprintf("\n");
        }
    }
}

void xxdf(void* ptr, uint32_t size)
{
    uint32_t i;
    uint32_t j;

    for(i = 0; i < size + ((size % DUMP_COLS) ? (DUMP_COLS - size % DUMP_COLS) : 0); i++) 
    {
        if(i % DUMP_COLS == 0) 
        {
            printf("0x%06x: ", i);
        }

        if(i < size) 
        {
            printf("%02x ", 0xFF & ((char*)ptr)[i]);
        }
        else
        {
            printf("   ");
        }

        if(i % DUMP_COLS == (DUMP_COLS - 1))
        {
            for(j = i - (DUMP_COLS - 1); j <= i; j++)
            {
                if(j >= size)
                {
                    printf(" ");
                }
                else if(isprint(((char*)ptr)[j]))
                {
                    printf("%c", (((char*)ptr)[j]) & 0xFF);
                }
                else
                {
                    printf(".");
                }
            }
            printf("\n");
        }
    }
}