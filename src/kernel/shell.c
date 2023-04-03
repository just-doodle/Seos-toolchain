#include "shell.h"

#include "debug.h"
#include "kheap.h"

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

void getCMD(char* cmd, char* help, shellcmdf_t function)
{
    memset(cmds[c_cmds].cmd, 0, 32);
    strcpy(cmds[c_cmds].cmd, cmd);
    strcpy(cmds[c_cmds].help, help);
    cmds[c_cmds].f = function;
    c_cmds++;
}

void init_shell()
{
    sbindx = 0;
    shell_buffer = kcalloc(sbsize, 1);

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
}

void clear_buffer()
{
    memset(shell_buffer, 0, sbsize);
    sbindx = 0;
}

void shell_f()
{
    shell_buffer[sbindx] = '\0';
    list_t *args = str_split(strdup(shell_buffer), " ", NULL);
    for(int i = 0; i < c_cmds; i++)
    {
        if(strcmp(list_get_node_by_index(args, 0)->val, cmds[i].cmd) == 0)
        {
            cmds[i].f(args);
            clear_buffer();
            printf("#/> ");
            return;
        }
    }

    clear_buffer();
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
        sbindx--;
    else
        shell_buffer[sbindx++] = c;
}