#include "system.h"
#include "multiboot.h"
#include "serial.h"
#include "vga_text.h"
#include "printf.h"

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

    printf("Hello World!\n");
    printf("The kernel verison is %s\n", KERNEL_VERSION);

    khalt;
}