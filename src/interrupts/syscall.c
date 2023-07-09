#include "syscall.h"
#include "process.h"
#include "stat.h"
#include "filedescriptor.h"
#include "compositor.h"
#include "serial.h"
#include "rtc.h"
#include "timer.h"
#include "mount.h"
#include "logdisk.h"
#include "dhcp.h"

int syscall_reboot()
{
    ldprintf("syscall", LOG_INFO, "Process %d, #%d [%s] has requested reboot of system.", current_process->pid, current_process->uid, current_process->filename);
    if(current_process->uid == USER_ID_ROOT)
    {
        ldprintf("syscall", LOG_WARN, "Rebooting system...");
        logdisk_dump("/kernelReboot.log");
        reboot();
        return 0;
    }
    else
    {
        return -1;
    }
}

int syscall_uname(struct utsname* name);

void * syscall_table[MAX_SYSCALL_ENTRIES] = {
    serial_putc, //0
    exit, //1
    attach_handler, //2
    fd_open, //3
    fd_close, //4
    fd_read, //5
    fd_write, //6
    fd_lseek, //7
    fstat, //8
    stat, //9
    vfs_unlink, //10
    getpid, //11
    kill, //12
    malloc, //13
    free, //14
    realloc, //15
    ksbrk, //16
    rand, //17
    process_get_ticks, //18
    execve, //19
    change_process, //20
    text_clear, //21
    get_args, //22
    create_window_from_info, //23
    window_display, //24
    register_wakeup_callback, //25
    sleep, //26
    window_change_title, //27
    fd_readdir, //28
    gettimeofday, //29
    window_swapBuffer, //30
    getcwd, //31
    chdir, //32
    fd_ioctl, //33
    umask, // 34
    access, //35
    syscall_uname, //36
    syscall_mount, //37
    sethostname, //38
    gethostname, //39
    getuid, //40
    setuid, //41
    syscall_reboot, //42
    chmod, //43
    vfs_mkdir, //44
};

void syscall_create_process(char* name, void* entrypoint)
{
    create_process_from_routine(name, entrypoint, TASK_TYPE_USER);
}

void syscall_handler(registers_t *reg)
{
    if(reg->eax >= MAX_SYSCALL_ENTRIES) return;
    void * system_api = syscall_table[reg->eax];
    if(validate(system_api) != 1)
    {
        reg->eax = -1;
        return;
    }
    int ret;
    //memcpy(&saved_context, reg, sizeof(registers_t));
    //serialprintf("SYSCALL 0x%02x\n", reg->eax);
    ASM_FUNC (" \
    push %1; \
    push %2; \
    push %3; \
    push %4; \
    push %5; \
    call *%6; \
    pop %%ebx; \
    pop %%ebx; \
    pop %%ebx; \
    pop %%ebx; \
    pop %%ebx; \
    " : "=a" (ret) : "r" (reg->edi), "r" (reg->esi), "r" (reg->edx), "r" (reg->ecx), "r" (reg->ebx), "r" (system_api));

    reg->eax = ret;
}

int register_syscall(void* handler, uint32_t syscode)
{
    if(validate(handler) != 1)
        return 1;

    if(validate(syscall_table[syscode]) == 1)
        return 1;

    if(syscode < MAX_KERNEL_SYSCALLS)
        return 1;

    if(syscode > MAX_SYSCALL_ENTRIES)
        return 1;

    syscall_table[syscode] = handler;
    ldprintf("Syscall manager", LOG_INFO, "New syscall handler has registered in 0x%x", syscode);
    return 0;
}

void init_syscall()
{
    register_interrupt_handler(0x80, syscall_handler);
}

int syscall_uname(struct utsname* name)
{
    if(validate(name) != 1)
        return -1;

    char versionNumber[_UTSNAME_LENGTH];
    sprintf(versionNumber, "%s", KERNEL_VERSION);
    char versionText[_UTSNAME_LENGTH];
    sprintf(versionText, "%s %s %s %s", KERNEL_VERSION_CODENAME, KERNEL_ENABLED_OPTIONS, KERNEL_BUILD_DATE, KERNEL_BUILD_TIME);

    strcpy(name->sysname, KERNEL_NAME);
    char* hname = zalloc(256);
    gethostname(hname);
    strcpy(name->nodename, hname);
    free(hname);
    strcpy(name->domainname, "");
    strcpy(name->machine, KERNEL_ARCH);
    strcpy(name->release, versionNumber);
    strcpy(name->version, versionText);
    return 0;
}