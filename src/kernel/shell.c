#include "shell.h"

#include "debug.h"
#include "kheap.h"
#include "vfs.h"
#include "mount.h"
#include "ramdisk.h"
#include "process.h"
#include "compositor.h"
#include "stdout.h"
#include "bitmap.h"
#include "modules.h"

shellcmd_t cmds[256];
int c_cmds = 0;

static char* shell_buffer;
int sbindx;
uint32_t sbsize = 512;

void register_command(shellcmd_t cmd)
{
    cmds[c_cmds++] = cmd;
}

int echo(list_t *args)
{
    uint32_t argc = list_size(args);
    for(int i = 1; i < argc; i++)
        printf("%s ", list_get_node_by_index(args, i)->val);
    printf("\n");
    return 0;
}

int Srand(list_t *args)
{
    printf("%d\n", abs(rand()));
    return 0;
}

int s_reboot(list_t *args)
{
    reboot();
    return 1;
}

int uname(list_t *args)
{
    int argc = list_size(args);
    if(argc < 2)
    {
        printf("SectorOS-RW4\n");
        return 1;
    }

    char* arg1 = list_pop(args)->val;
    if(strcmp(arg1, "-a") == 0)
    {
        printf("SectorOS-RW4 %s i386\n", KERNEL_VERSION);
    }
    else if(strcmp(arg1, "-p") == 0)
    {
        cpuinfo_t* cpu = get_cpuinfo();
        printf("%s\n", cpu->infoString);
    }
    else if(strcmp(arg1, "-v") == 0)
    {
        printf("%s\n", KERNEL_VERSION);
    }
    else if(strcmp(arg1, "-s") == 0)
    {
        printf("SectorOS-RW4\n");
    }
    else if(strcmp(arg1, "-h") == 0)
    {
        printf("uname [options]\nPrints specific information about system\nOptions:\n\t-a: Prints all information.\n\t-p: Prints processor name\n\t-v: Prints Kernel version\n\t-s: Prints kernel name\n\t-h: Prints this message\n");
    }
    return 0;
}

int ls(list_t *args)
{
    vfs_db_listdir(list_get_node_by_index(args, 1)->val);
    return 0;
}

int s_arpb(list_t* args)
{
    char* ip_str = list_pop(args)->val;
    list_t* ip_lst = str_split(ip_str, ".", NULL);

    uint8_t* buf = zalloc(sizeof(uint8_t)*4);

    int h = 0;

    foreach(k, ip_lst)
    {
        buf[h] = k->val;
        h++;
    }

    arp_broadcast(buf);
    free(buf);
    return 0;
}

int s_dufunc(char* fname)
{
    FILE* f = file_open(fname, OPEN_RDONLY);
    if(f == NULL || strcmp(f->name, "/") == 0)
    {
        printf("File not found: %s\n", fname);
        return 1;
    }

    size_t sz = vfs_getFileSize(f);
    printf("%s: %dB\n", f->name, sz);
    vfs_close(f);
    return 0;

}

int s_diskusage(list_t* args)
{
    uint32_t argc = list_size(args);
    if(argc < 2)
    {
        printf("usage: %s [file]\n", list_get_node_by_index(args, 0)->val);
        return 1;
    }

    char* fname = strdup(list_get_node_by_index(args, 1)->val);
    return s_dufunc(fname);
}

int s_readfun(size_t sz, char* fname)
{
    FILE* f = file_open(fname, OPEN_RDONLY);
    if(f == NULL || strcmp(f->name, "/") == 0)
    {
        printf("File not found: %s\n", fname);
        return 1;
    }

    char* buffer = zalloc(sz);
    vfs_read(f, 0, sz, buffer);
    for(int i = 0; i < sz; i++)
        text_putc(buffer[i]);
    printf("\n");
    free(buffer);
    vfs_close(f);
    return 0;
}

int s_ramdisk(list_t* args)
{
    uint32_t argc = list_size(args);
    if(argc < 2)
    {
        printf("usage: %s [file]\n");
        return 1;
    }

    char* name = list_pop(args)->val;
    add_ramdisk_file(name, 1);
    return 0;
}

int s_read(list_t* args)
{
    uint32_t argc = list_size(args);
    if(argc < 3)
    {
        printf("usage: %s [file] [size]\n");
        return 1;
    }

    uint32_t sz = atoi(list_pop(args)->val);
    char* fname = list_pop(args)->val;
    return s_readfun(sz, fname);
}

int s_dPrintVFS(list_t* args)
{
    print_vfs_tree();
    return 0;
}

int s_mount(list_t* args)
{
    uint32_t argc = list_size(args);

    if(argc < 3)
        return 1;

    char* mountpoint = list_pop(args)->val;
    char* device = list_pop(args)->val;

    return mount(device, mountpoint);  
}

int s_chcolor(list_t* args)
{
    uint32_t argc = list_size(args);
    if(argc < 3)
        return 1;

    uint8_t fg = atoi(list_pop(args)->val) & 0xFF;
    uint8_t bg = atoi(list_pop(args)->val) & 0xFF;

    serialprintf("[COLOR] 0x%02x, 0x%02x\n", bg, fg);

    text_chcolor(fg, bg);
    return 0;
}

int help(list_t *args)
{
    uint32_t argc = list_size(args);

    if(argc < 2)
    {
        printf("Commands:\n");
        for(int i = 0; i < c_cmds; i++)
            printf("%s ", cmds[i].cmd);

        printf("\nRun help [command] to get more info about given command.\n");
        return 1;
    }

    for(int i = 0; i < c_cmds; i++)
    {
        if(strcmp(list_get_node_by_index(args, 1)->val, cmds[i].cmd) == 0)
        {
            printf("%s: %s\n", cmds[i].cmd, cmds[i].help);
            return 0;
        }
    }
    printf("Commands:\n");
    for(int i = 0; i < c_cmds; i++)
        printf("%s ", cmds[i].cmd);
    printf("\nRun help [command] to get more info about given command.\n");

    return 1;
}

int s_textclear(list_t* args)
{
    text_clear();
    return 0;
}


int s_showimg(list_t* args)
{
    uint32_t argc = list_size(args);
    if(argc < 2)
    {
        printf("usage: %s [file]\n", list_get_node_by_index(args, 0)->val);
        return 1;
    }
    char* filename = list_pop(args)->val;
    window_show_image(filename);
    return 0;
}

int s_showbmp(list_t* args)
{
    uint32_t argc = list_size(args);
    if(argc < 2)
    {
        printf("usage: %s [file]\n", list_get_node_by_index(args, 0)->val);
        return 1;
    }
    char* filename = list_pop(args)->val;
    bitmap_t* b = bitmap_create(filename);

    if(b->width > vesa_getXResolution() || b->height > vesa_getYResolution())
    {
        return 1;
    }

    window_t* w = create_window(filename, b->width, b->height, 32, 32);

    uint32_t* fb = zalloc(w->fb_size);

    if(b->bpp == 24)
        bitmap_to_framebuffer(b, fb);
    else
        bitmap_to_framebuffer2(b, fb);

    window_display(w, fb);
    free(fb);

    return 0;
}

int s_insmod(list_t* args)
{
    uint32_t argc = list_size(args);
    if(argc < 2)
    {
        printf("usage: %s [file] params...\n", list_get_node_by_index(args, 0)->val);
        return 1;
    }

    char** argv = zalloc(sizeof(uint32_t)*argc);
    int j;
    for(j = 1; j < argc; j++)
    {
        listnode_t* l = list_get_node_by_index(args, j);
        argv[j-1] = zalloc(strlen(l->val));
        strcpy(argv[j-1], l->val);
    }
    argv[j++] = NULL;

    return load_module(argv);
}

int s_rmmod(list_t* args)
{
    uint32_t argc = list_size(args);
    if(argc < 2)
    {
        printf("usage: %s [name]\n", list_get_node_by_index(args, 0)->val);
        return 1;
    }

    char* name = list_get_node_by_index(args, 1)->val;
    return unload_module(name);
}

int s_lsmod(list_t* args)
{
    print_list_modules();
    return 0;
}

int s_elf(list_t* args)
{
    uint32_t argc = list_size(args);
    if(argc < 2)
    {
        printf("usage: %s [file]\n", list_get_node_by_index(args, 0)->val);
        return 1;
    }

    char* file_name = list_get_node_by_index(args, 1)->val;
    char** argv = zalloc(sizeof(uint32_t)*argc);
    int j;
    for(j = 2; j < argc; j++)
    {
        listnode_t* l = list_get_node_by_index(args, j);
        argv[j-2] = zalloc(strlen(l->val));
        strcpy(argv[j-2], l->val);
    }
    argv[j++] = NULL;
    execve(file_name, argv, NULL);
    return 0;
}

void getCMD(char* cmd, char* help, shellcmdf_t function)
{
    memset(cmds[c_cmds].cmd, 0, 32);
    strcpy(cmds[c_cmds].cmd, cmd);
    strcpy(cmds[c_cmds].help, help);
    cmds[c_cmds].f = function;
    c_cmds++;
}

int s_cat(list_t* args)
{
    uint32_t argc = list_size(args);
    if(argc < 2)
    {
        printf("usage: %s [file]\n", list_get_node_by_index(args, 0)->val);
        return 1;
    }

    char* file_name = list_pop(args)->val;
    FILE* f = file_open(file_name, 0);
    if(f == NULL)
        return 1;
    uint32_t sz = vfs_getFileSize(f);
    if(sz == 0)
        sz = f->size;
    char* buf = zalloc(sz);
    vfs_read(f, 0, sz, buf);
    printf("%s\n", buf);
    free(buf);
    return 0;
}

int s_loglevel(list_t* args)
{
    uint32_t argc = list_size(args);
    if(argc < 2)
    {
        printf("usage: %s [log_level=(0, 1, 2, 3, 4, 5)]\n", list_get_node_by_index(args, 0)->val);
        return 1;
    }

    char* loglevel_c = list_pop(args)->val;
    int loglevel = atoi(loglevel_c);

    logdisk_change_policy(loglevel);
    return 0;
}

void init_shell()
{
    sbindx = 0;
    shell_buffer = zalloc(sbsize);

    change_keyboard_handler(shell_callback);

    shellcmd_t echocmd;
    memset(echocmd.cmd, 0, 32);
    strcpy(echocmd.cmd, "echo\0");
    strcpy(echocmd.help, "Echos back given string.");
    echocmd.f = echo;
    register_command(echocmd);

    shellcmd_t helpcmd;
    memset(helpcmd.cmd, 0, 32);
    strcpy(helpcmd.cmd, "help");
    strcpy(helpcmd.help, "Returns info about given command.");
    helpcmd.f = help;
    register_command(helpcmd);

    getCMD("rand", "Returns a random number", Srand);
    getCMD("reboot", "Reboots the system", s_reboot);
    getCMD("uname", "Prints info about the kernel. For more information run this command with -h flag", uname);
    getCMD("ls", "Lists all files in given directory", ls);
    getCMD("clear", "Clears the screen.", s_textclear);
    getCMD("mount", "Mounts the given device to mountpoint", s_mount);
    getCMD("du", "Prints the size of given file.", s_diskusage);
    getCMD("read", "Prints the content of file for given size.", s_read);
    getCMD("debug_printVFS", "Prints the VFS table", s_dPrintVFS);
    getCMD("create_ramdisk", "Creates ramdisk of given file in /dev/ directory", s_ramdisk);
    getCMD("color", "Changes the text color. Only use decimal numbers.", s_chcolor);
    getCMD("loadelf", "Loads the given ELF file.", s_elf);
    getCMD("showbmp", "Shows the given bitmap image.", s_showbmp);
    getCMD("showimg", "Shows the given image.", s_showimg);
    getCMD("cat", "Prints the content of the file", s_cat);
    getCMD("insmod", "Installs the given module to kernel", s_insmod);
    getCMD("rmmod", "Unloads the module with given name", s_rmmod);
    getCMD("lsmod", "Lists the loaded modules", s_lsmod);
    getCMD("arpb", "Broadcast ip to given ip address", s_arpb);
    getCMD("log_level", "Sets the level of log message to be shown.", s_loglevel);
}

void clear_buffer()
{
    memset(shell_buffer, 0, sbsize);
    sbindx = 0;
}

uint32_t k = 0;

void shell_f()
{
    shell_buffer[sbindx] = '\0';
    list_t *args = str_split(strndup(shell_buffer, sbindx), " ", NULL);
    for(int i = 0; i < c_cmds; i++)
    {
        if(strcmp(list_get_node_by_index(args, 0)->val, cmds[i].cmd) == 0)
        {
            shellcmdf_t f = cmds[i].f;
            f(args);
            clear_buffer();
            printf("kshell #> ");
            return;
        }
        else if(strncmp(list_get_node_by_index(args, 0)->val, "/", 1) == 0)
        {
            char* file_name = list_get_node_by_index(args, 0)->val;
            char** argv = zalloc(sizeof(uint32_t)*list_size(args));
            int j;
            for(j = 1; j < list_size(args); j++)
            {
                listnode_t* l = list_get_node_by_index(args, j);
                argv[j-1] = zalloc(strlen(l->val));
                strcpy(argv[j-1], l->val);
            }
            argv[j++] = NULL;
            execve(file_name, argv, NULL);
            printf("kshell #> ");
        }
    }

    printf("Unknown command: %s\n", list_get_node_by_index(args, 0)->val);
    clear_buffer();
    printf("#/> ");
}

void shell_callback(uint8_t scancode)
{
    if(scancode == 0x3E)
        xxd(shell_buffer, sbsize);
    if(sbindx > sbsize)
        clear_buffer();

    char c = kcodeTochar(scancode);
    printf("%c", c);
    if(c == 0)
        return;
    if(c == '\n')
    {
        shell_f();
        return;
    }
    if(c == '\b')
        shell_buffer[sbindx--] = 0;
    else
        shell_buffer[sbindx++] = c;
}