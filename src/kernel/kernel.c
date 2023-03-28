#include "system.h"
#include "multiboot.h"
#include "serial.h"
#include "vga_text.h"
#include "printf.h"
#include "gdt.h"
#include "idt.h"
#include "pit.h"



void kernelmain(const multiboot_info_t* info, uint32_t multiboot_magic)
{
    init_serial(COM1, 1);

    init_text();
    text_chcolor(VGA_LIGHT_GREEN, VGA_BLACK);
    text_clear();

    if(multiboot_magic != 0x2BADB002)
    {
        serial_puts("multiboot bootloader magic is invalid. Kernel cannot boot.\n");
        return;
    }

    if(info->flags & (1<<5))
    {
        serial_puts("ELF_SHDR flag is set\n");
    }

    printf("Multiboot info:\n\t-Bootloader: %s\n\t-CmdLine: %s\n\t-Available memory: %dMB\n", info->boot_loader_name, info->cmdline, (info->mem_upper / 1024));

    init_gdt();
    init_idt();

    init_pic();

    enable_interrupts();

    init_pit();
    printf("[KERNEL] Kernel has successfully initialized\n\n");

    printf("Hello World!\n");
    sleep(1000);
    printf("The kernel version is %s\n", KERNEL_VERSION);

    khalt;
}