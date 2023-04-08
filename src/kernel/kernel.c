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
#include "sorfs.h"
#include "shell.h"
#include "atapio.h"
#include "ramdisk.h"
#include "kernelfs.h"
#include "pci.h"
#include "vfs.h"
#include "devfs.h"
#include "mount.h"

#define MOTD_NUM 4

void print_motd(int idx)
{
    switch(idx)
    {
    case 1:
        printf("   _____           _              ____   _____       _______          ___  _   \n  / ____|         | |            / __ \\ / ____|     |  __ \\ \\        / / || |  \n | (___   ___  ___| |_ ___  _ __| |  | | (___ ______| |__) \\ \\  /\\  / /| || |_ \n  \\___ \\ / _ \\/ __| __/ _ \\| '__| |  | |\\___ \\______|  _  / \\ \\/  \\/ / |__   _|\n  ____) |  __/ (__| || (_) | |  | |__| |____) |     | | \\ \\  \\  /\\  /     | |  \n |_____/ \\___|\\___|\\__\\___/|_|   \\____/|_____/      |_|  \\_\\  \\/  \\/      |_|  \n                                                                               \n                                                                               \n");
        break;
    case 2:
        printf("   _____           __             ____ _____       ____ _       ____ __\n  / ___/___  _____/ /_____  _____/ __ / ___/      / __ | |     / / // /\n  \\__ \\/ _ \\/ ___/ __/ __ \\/ ___/ / / \\__ \\______/ /_/ | | /| / / // /_\n ___/ /  __/ /__/ /_/ /_/ / /  / /_/ ___/ /_____/ _, _/| |/ |/ /__  __/\n/____/\\___/\\___/\\__/\\____/_/   \\____/____/     /_/ |_| |__/|__/  /_/   \n                                                                       \n");
        break;
    case 3:
        printf("   _____           __             ____ _____       ____ _       ____ __\n  / ___/___  _____/ /_____  _____/ __ / ___/      / __ | |     / / // /\n  \\__ \\/ _ \\/ ___/ __/ __ \\/ ___/ / / \\__ \\______/ /_/ | | /| / / // /_\n ___/ /  __/ /__/ /_/ /_/ / /  / /_/ ___/ /_____/ _, _/| |/ |/ /__  __/\n/____/\\___/\\___/\\__/\\____/_/   \\____/____/     /_/ |_| |__/|__/  /_/   \n                                                                       \n");
    default:
        printf("SectorOS-RW4\n");
        break;
    };
}

void rand_motd()
{
    int r = rand_range(0, MOTD_NUM);
    print_motd(r);
}

void kernelmain(const multiboot_info_t* info, uint32_t multiboot_magic)
{
    init_serial(COM1, 1);

    init_text();
    text_chcolor(VGA_WHITE, VGA_LIGHT_BLUE);
    text_clear();

    init_pmm(info->mem_upper * 1024);
    init_paging();
    init_kheap(KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE, KHEAP_MAX_ADDRESS);

    init_gdt();
    init_idt();
    init_pic();

	init_pit();

    enable_interrupts();

    init_shell();
    init_keyboard();

    init_pci();

    init_vfs();
    init_devfs();
    init_ata_pio();

    printf("Multiboot info:\n\t-Bootloader: %s\n\t-CmdLine: %s\n\t-Available memory: %dMB\n", info->boot_loader_name, info->cmdline, (info->mem_upper / 1024));

    printf("[KERNEL] Kernel has successfully initialized\n\n");

    printf("Hello World!\n");
    printf("The kernel version is %s\n", KERNEL_VERSION);

    init_rand();

    char* test = "Hello User!\n";

    init_kernelfs();
    kernelfs_add_variable("test", test, strlen(test), KERNELFS_TYPE_STRING);

    print_cpu_info();

    if(info->mods_count > 0)
    {
        printf("[MULTIBOOT] mods count: %d\n", info->mods_count);
        multiboot_module_t* mods = (multiboot_module_t*)info->mods_addr;
        printf("[MULTIBOOT] mods name: %s\n", mods[0].cmdline);
        alloc_region(kernel_page_dir, mods[0].mod_start, mods[0].mod_end, 1, 1, 1);
        serialprintf("Ramdisk created\n");
        add_ramdisk(mods[0].mod_start, mods[0].mod_end, 1);
    }

    printf("rand: ");
    for(int i = 0; i < 10; i++)
        printf("%d ", rand_range(0, 256));

    text_clear();

    rand_motd();
    printf("Version: %s\n", KERNEL_VERSION);
    printf("\n");
    if(isRamdiskCreated(0))
    {
        printf("[KERNEL] One module is loaded by the bootloader. The module can be accessed via /dev/rdisk0.\nTo mount the drive: mount /dev/rdisk0 [Mountpoint].\n");
    }
    printf("\nSectorOS Kernel v1.3.2\nRun help to get the list of commands.\n#/> ");

    while(1);
}