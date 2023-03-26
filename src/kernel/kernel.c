#include "system.h"
#include "multiboot.h"
#include "serial.h"
#include "vga_text.h"

void kernelmain(const multiboot_info_t* info, uint32_t multiboot_magic)
{
    init_serial(COM1, 1);

    init_text();
    text_chcolor(VGA_LIGHT_GREEN, VGA_BLACK);
    text_clear();

    if(multiboot_magic != 0x2BADB002)
    {
        serial_puts("multiboot bootloader magic is invalid\n");
        return;
    }

    text_puts("Hello World!\n");
    text_puts("multiboot bootloader magic is valid...\n");
    text_puts("Kernel version is: ");
    text_puts(KERNEL_VERSION);
    text_putc('\n');

    khalt;
}