#include "system.h"
#include "multiboot.h"

void kernelmain(const multiboot_info_t* info, uint32_t multiboot_magic)
{
    if(multiboot_magic != 0x2BADB002)
    {
        return;
    }
    reboot();
}