#include "system.h"
#include "ports.h"

void reboot()
{
    outb(0x64, 0xFE);
}