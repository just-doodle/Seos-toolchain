#include "rdtsc.h"

ul_t calculate_tsc()
{
    ul_t st_high = 0;
    ul_t st_low = 0;
    ul_t end_high = 0;
    ul_t end_low = 0;
    ul_t count = 0;

    outb(0x61, (inb(0x61) & ~0x02) | 0x01);
    outb(PIT_COMMAND, 0xB0);
    outb(PIT_CHANNEL2, calibrate_tsc & 0xFF);
    outb(PIT_CHANNEL2, (calibrate_tsc >> 8) & 0xFF);
    readtsc(st_low, st_high);
    while((inb(0x61) & 0x20) == 0)
    {
        count++;
    }

    readtsc(end_low, end_high);

    if(count <= 1)
        return 0;

    ASM_FUNC ("subl %2, %0\n"
	    "sbbl %3,%1":"=a" (end_low),"=d"(end_high):"g"(st_low),"g"(st_high),"0"(end_low),"1"(end_high));
    if(end_high != 0)
    {
        return 0;
    }
    
    if(end_low <= calibrate_time)
    {
        return 0;
    }
	ASM_FUNC ("divl %2":"=a"(end_low),"=d"(end_high):"r"(end_low),"0"(0),"1"(calibrate_time));
	return end_low;
}