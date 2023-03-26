#include "power.h"

void reboot()
{
    outb(0x64, 0xFE);
    khalt;
}