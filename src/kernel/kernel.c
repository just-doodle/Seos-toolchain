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
#include "cpuinfo.h"
#include "keyboard.h"
#include "shell.h"

void kernelmain(const multiboot_info_t* info, uint32_t multiboot_magic)
{
    init_serial(COM1, 1);

    init_text();
    text_chcolor(VGA_LIGHT_GREEN, VGA_BLACK);
    text_clear();

    init_gdt();
    init_idt();

    init_pic();

    init_pmm(1024 * MB);
    init_paging();
    init_kheap(KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE, KHEAP_MAX_ADDRESS);

	init_pit();

    enable_interrupts();

    init_shell();
    init_keyboard();

    printf("Multiboot info:\n\t-Bootloader: %s\n\t-CmdLine: %s\n\t-Available memory: %dMB\n", info->boot_loader_name, info->cmdline, (info->mem_upper / 1024));

    printf("[KERNEL] Kernel has successfully initialized\n\n");

    printf("Hello World!\n");
    printf("The kernel version is %s\n", KERNEL_VERSION);

    init_rand();

    print_cpu_info();

    printf("rand: ");
    for(int i = 0; i < 10; i++)
        printf("%d ", rand_range(0, 256));

    printf("\n\n");
    printf("SectorOS Kernel v1.0.0\nRun help to get the list of commands.\n#/> ");

    while(1);
}