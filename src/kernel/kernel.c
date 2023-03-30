#include "system.h"
#include "serial.h"
#include "vga_text.h"
#include "printf.h"
#include "gdt.h"
#include "idt.h"
#include "pit.h"
#include "paging.h"
#include "kheap.h"
#include "pmm.h"

void kernelmain(const multiboot_info_t* info, uint32_t multiboot_magic)
{
    init_serial(COM1, 1);

    init_text();
    text_chcolor(VGA_LIGHT_GREEN, VGA_BLACK);
    text_clear();

    init_gdt();
    init_idt();

    init_pic();

    m_info = info;

    init_pmm(1096 * MB);
    init_paging();
    init_kheap(KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE, KHEAP_MAX_ADDRESS);

	init_symDB();

	init_pit();

    printf("Multiboot info:\n\t-Bootloader: %s\n\t-CmdLine: %s\n\t-Available memory: %dMB\n", info->boot_loader_name, info->cmdline, (info->mem_upper / 1024));

    enable_interrupts();

    printf("[KERNEL] Kernel has successfully initialized\n\n");

    printf("Hello World!\n");
    printf("The kernel version is %s\n", KERNEL_VERSION);

    backtrace();

    khalt;
}