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
#include "tss.h"
#include "usermode.h"
#include "syscall.h"
#include "process.h"

#define MOTD_NUM 3

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
        break;
    default:
        printf("SectorOS-RW4\n");
        break;
    };
}

void usp2()
{
    SYSCALL_CLEAR
    SYSCALL_PUTCHAR('W');
    SYSCALL_EXIT(0)
}

void usp()
{
    SYSCALL_CLEAR
    SYSCALL_PUTCHAR('H');
    SYSCALL_CREATE_PROCESS_FUN("USP2", usp2)
    while(1);
}


void rand_motd()
{
    int r = rand_range(0, MOTD_NUM);
    print_motd(r);
}

list_t* mboot_cmd;

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
    init_tss(0x05, 0x10, 0);

    init_pic();

	init_pit();

    init_syscall();

    init_processManager();

    mboot_cmd = str_split(((char*)info->cmdline), " ", NULL);

    enable_interrupts();
    init_keyboard();

    init_pci();

    init_vfs();
    init_devfs();
    init_ata_pio();

    uint32_t esp;
    asm volatile("mov %%esp, %0" : "=r"(esp));
    tss_set_kernel_stack(0x10, esp);

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
        xxd((void*)mods[0].mod_start, (mods[0].mod_end - mods[0].mod_start));
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

    int j = 0;

    if((j = list_contain_str(mboot_cmd, "--root")) != -1)
    {
        if(j > list_size(mboot_cmd))
        {
            printf("[KERNEL] Device path is not in arguments.\n");
        }
        else
        {
            char* root = (list_get_node_by_index(mboot_cmd, j+1)->val);
            mount(root, "/");
        }
    }

    init_shell();
    printf("\nSectorOS shell v1.3.3\nRun help to get the list of commands.\n#/> ");

    while(1);
}