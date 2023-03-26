#include "system.h"
#include "multiboot.h"
#include "serial.h"

void kernelmain(const multiboot_info_t* info, uint32_t multiboot_magic)
{
    init_serial(COM1, 0);
    if(multiboot_magic != 0x2BADB002)
    {
        serial_puts("multiboot bootloader magic is invalid\n");
        return;
    }
    serial_puts("multiboot bootloader magic is valid...\n");
    serial_puts("Kernel version is:");
    serial_puts(KERNEL_VERSION);
    serial_putc('\n');
    reboot();
}