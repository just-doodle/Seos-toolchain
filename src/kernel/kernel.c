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
#include "stdout.h"
#include "sse.h"
#include "vesa.h"
#include "ifb.h"
#include "compositor.h"
#include "timer.h"
#include "tmpfs.h"
#include "portdev.h"
#include "nulldev.h"
#include "mount.h"
#include "logdisk.h"

#include "rtl8139.h"
#include "pcnet.h"
#include "arp.h"
#include "ipv4.h"
#include "udp.h"
#include "tcp.h"
#include "dhcp.h"

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
    SYSCALL_PUTC('W');
    SYSCALL_EXIT(0)
}

void usp()
{
    SYSCALL_CLEAR
    SYSCALL_PUTC('H');
    while(1);
}


void rand_motd()
{
    int r = rand_range(0, MOTD_NUM);
    print_motd(r);
}

char* mount_filesystems = "nodev	tmpfs\n        sorfs\n";

list_t* mboot_cmd;

int no_process = 0;

void sample_callback(ipv4_addr_t* addr, uint8_t* data, uint32_t len)
{
    ldprintf("kernel", LOG_DEBUG, "DATA FROM %d.%d.%d.%d:\n\t%s\n", addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3], data);
}

int tcp_sample(TCPSocket_t* self, uint8_t* data, uint16_t len)
{
    uint8_t addr[8];
    iptoa(self->remIP, addr);
    ldprintf("kernel", LOG_DEBUG, "DATA FROM %d.%d.%d.%d:\n\t%s\n", addr[0], addr[1], addr[2], addr[3], data);
    
    if (len > 9 && data[0] == 'G' && data[1] == 'E' && data[2] == 'T' && data[3] == ' ' && data[4] == '/' && data[5] == ' ' && data[6] == 'H' && data[7] == 'T' && data[8] == 'T' && data[9] == 'P')
    {
        ldprintf("KERNEL", LOG_DEBUG, "GOT CONNECTION OVER HTTP");
        tcpsocket_send(self, (uint8_t *)"HTTP/1.1 200 OK\r\nServer: SectorOS\r\nContent-Type: text/html\r\n\r\n<html><head><title>SectorOS</title></head><body><b>This is a test webpage which is hosted on SectorOS</b> https://github.com/Arun007coder/SectorOS</body></html>\r\n", 224);
        tcp_disconnect(self);
    }
    return 1;
}

void kernelmain(const multiboot_info_t* info, uint32_t multiboot_magic)
{
    init_serial(COM1, 1);

    init_text();
    text_chcolor(VGA_WHITE, VGA_LIGHT_BLUE);
    text_clear();

    init_gdt();
    init_idt();

    int logidx;

    init_pmm(1024 * info->mem_upper);
    init_paging();
    init_kheap(KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE, KHEAP_MAX_ADDRESS);
    init_logdisk(4*MB, LOG_ERR);

    ldprintf("kernel", LOG_INFO, "Running SectorOS-RW4 kernel %s", KERNEL_VERSION);
    ldprintf("multiboot", LOG_INFO, "cmdline: %s", info->cmdline);
    ldprintf("gdt", LOG_INFO, "Added %d entries to GDT", GDT_MAX_ENTRIES);
    ldprintf("idt", LOG_INFO, "IDT has been initialized");
    ldprintf("pmm", LOG_INFO, "Memory found in system: %dKB", ((info->mem_upper)));
    ldprintf("pmm", LOG_INFO, "Physical memory manager has been initialized");
    ldprintf("paging", LOG_INFO, "Memory page manager has been initialized");
    ldprintf("kernel_heap", LOG_INFO, "Heap start: 0x%x, Heap end: 0x%x, Heap max address: 0x%x, Heap initial size: %dB", KHEAP_START, (KHEAP_START+KHEAP_INITIAL_SIZE), KHEAP_MAX_ADDRESS, KHEAP_INITIAL_SIZE);
    ldprintf("kernel_heap", LOG_INFO, "Kernel heap manager has been initialized");

    init_pic();

    init_vfs();
    init_devfs();
    logdisk_mount();

    mboot_cmd = str_split(((char*)info->cmdline), " ", NULL);

    if((logidx = list_contain_str(mboot_cmd, "--loglevel")) != 0)
    {
        if(list_size(mboot_cmd) < (logidx+1))
        {
            ldprintf("kernel", LOG_WARN, "Argument %s does not have a value, therefore this command will be ignored.", list_get_node_by_index(mboot_cmd, logidx)->val);
        }
        else
        {
            listnode_t* l = list_get_node_by_index(mboot_cmd, logidx+1);
            uint32_t logstate = atoi(l->val);
            logdisk_change_policy(logstate);
        }
    }

    init_mount();
    init_kernelfs("/proc");
    kernelfs_add_variable("filesystems", mount_filesystems, strlen(mount_filesystems));
    syscall_mount(NULL, "/tmp", "tmpfs", 0, NULL);

    init_portDev();
    init_nulldev();

    init_timer_interface();
	init_pit();

    enable_sse();
    enable_fast_memcpy();

    init_vesa(info);

    init_ifb();

    init_compositor();

    init_seterm();

    init_syscall();

    int q = 0;
    if((q = list_contain_str(mboot_cmd, "--no_process")) != -1)
    {
        no_process = 1;
    }

    if(no_process == 0)
        init_tss(0x05, 0x10, 0);

    if(no_process == 0)
        init_processManager();

    init_keyboard();

    init_pci();
    
    init_ata_pio();
    init_stdout();

    uint32_t esp;
    asm volatile("mov %%esp, %0" : "=r"(esp));
    if(no_process == 0)
        tss_set_kernel_stack(0x10, esp);

    ldprintf("multiboot", LOG_DEBUG, "\t-Bootloader: %s\n\t-CmdLine: %s\n\t-Available memory: %dMB\n", info->boot_loader_name, info->cmdline, (info->mem_upper / 1024));

    ldprintf("KERNEL", LOG_INFO, "Kernel has successfully initialized");

    init_rand();

    //print_cpu_info();

    ldprintf("Rand", LOG_DEBUG, "%d %d %d %d %d %d %d %d %d %d\n", rand_range(0, 256), rand_range(0, 256), rand_range(0, 256), rand_range(0, 256), rand_range(0, 256), rand_range(0, 256), rand_range(0, 256), rand_range(0, 256), rand_range(0, 256), rand_range(0, 256));

    text_clear();

    rand_motd();
    printf("Version: %s\n", KERNEL_VERSION);
    printf("\n");

    int j = 0;

    if((j = list_contain_str(mboot_cmd, "--root")) != -1)
    {
        if(j > list_size(mboot_cmd))
        {
            ldprintf("Kernel", LOG_ERR, "Device path is not in arguments.");
        }
        else
        {
            char* root = (list_get_node_by_index(mboot_cmd, j+1)->val);
            mount(root, "/");
        }
    }

    enable_interrupts();

    ldprintf("Kernel", LOG_WARN, "The page directory of the processes will not be freed because of a bug. This will cause memory leak.");

    printtime();

    compositor_load_wallpaper("/wallpaper.bmp", 2);
    init_shell();

    print_mountList();

    if(list_contain_str(mboot_cmd, "--network_enable_loopback") != -1)
    {
        init_networkInterfaceManager();
        init_loopback();
        init_rtl8139();
        init_pcnet(); //! Does not work

        switch_interface(0);

        read_mac_addr();

        init_arp();
        init_udp();
        init_dhcp();

        init_tcp();

        // dhcp_discover();
        // while(isIPReady() == 0);

        TCPSocket_t* s = tcp_listen(9090);
        tcp_bind(s, tcp_sample);

        uint8_t ip[4];
        get_ip2(ip);

        TCPSocket_t* sock = tcp_connect(ip, 9090);
        tcpsocket_send(sock, "GET / HTTP", 11);
    }

    if(list_contain_str(mboot_cmd, "--network_enable") != -1)
    {
        init_networkInterfaceManager();
        init_loopback();
        init_rtl8139();
        init_pcnet(); //! Does not work

        switch_interface(1);

        read_mac_addr();

        init_arp();
        init_udp();
        init_dhcp();

        init_tcp();

        dhcp_discover();
        while(isIPReady() == 0);

        TCPSocket_t* s = tcp_listen(9090);
        tcp_bind(s, tcp_sample);
    }

    printf("\nSectorOS shell v2.0.0\nRun help to get the list of commands.\n#/> ");
    while(1);
}