#include "mount.h"
#include "sorfs.h"
#include "vfs.h"
#include "tmpfs.h"
#include "process.h"
#include "ext2.h"
#include "kernelfs.h"

list_t* mount_list = NULL;

int mount(char* devpath, char* mountpoint)
{
    uint32_t t = find_fs(devpath);
    if(t == FS_TYPE_SORFS)
    {
        init_sorfs(devpath, mountpoint);
        mount_info_t* m = ZALLOC_TYPES(mount_info_t);
        m->device = strdup(devpath);
        m->mountpoint = strdup(mountpoint);
        m->fs_type = t;
        list_push(mount_list, m);
        return 0;
    }
    else if(t == FS_TYPE_EXT2)
    {
        init_ext2(devpath, mountpoint);
        mount_info_t* m = ZALLOC_TYPES(mount_info_t);
        m->device = strdup(devpath);
        m->mountpoint = strdup(mountpoint);
        m->fs_type = t;
        list_push(mount_list, m);
        return 0;
    }
    // else if (t == FS_TYPE_EXT2)
    // {
    //     init_ext2(devpath, mountpoint);
    //     return 0;
    // }
    
    else
    {
        return -1;
    }
}

int mount_specifyFS(char* devpath, char* mountpoint, uint32_t t)
{
    if(t == FS_TYPE_SORFS)
    {
        init_sorfs(devpath, mountpoint);
        mount_info_t* m = ZALLOC_TYPES(mount_info_t);
        m->device = strdup(devpath);
        m->mountpoint = strdup(mountpoint);
        m->fs_type = t;
        list_push(mount_list, m);
        return 0;
    }
    else if(t == FS_TYPE_EXT2)
    {
        init_ext2(devpath, mountpoint);
        mount_info_t* m = ZALLOC_TYPES(mount_info_t);
        m->device = strdup(devpath);
        m->mountpoint = strdup(mountpoint);
        m->fs_type = t;
        list_push(mount_list, m);
        return 0;
    }
    else if(t == FS_TYPE_TMPFS)
    {
        init_tmpfs(mountpoint);
        return 0;
    }
    else if(t == FS_TYPE_KERNELFS)
    {
        init_kernelfs(mountpoint);
        return 0;
    }
    else
    {
        return -1;
    }
}

char* fs_name(uint32_t fs_type)
{
    switch(fs_type)
    {
    case FS_TYPE_EXT2:
        return "EXT2";
        break;
    case FS_TYPE_SORFS:
        return "SORFS";
        break;
    case FS_TYPE_DEVFS:
        return "DEVFS";
        break;
    case FS_TYPE_TMPFS:
        return "TMPFS";
        break;
    case FS_TYPE_KERNELFS:
        return "KERNELFS";
        break;
    default:
        return "Unknown FS";
        break;
    };
}

void print_mountList()
{
    foreach(l, mount_list)
    {
        mount_info_t* m = l->val;
        printf("%s -> %s [%s]\n", m->device, m->mountpoint, fs_name(m->fs_type));
    }
}

int syscall_mount(const char* device, const char* target, const char* filesystem_type, int mount_flags, char* fsflags)
{
    if(current_process->uid != USER_ID_ROOT)
        return -1;

    if(validate(target) != 1)
        return -1;

    if((device != NULL) && (validate(device) != 1))
        return -1;

    if(validate(filesystem_type) != 1)
        return -1;


    if(strcmp(filesystem_type, "sorfs") == 0)
    {
        mount_specifyFS(device, target, FS_TYPE_SORFS);
        return 0;
    }
    else if(strcmp(filesystem_type, "ext2") == 0)
    {
        mount_specifyFS(device, target, FS_TYPE_EXT2);
        return 0;
    }
    else if(strcmp(filesystem_type, "tmpfs") == 0)
    {
        mount_specifyFS(device, target, FS_TYPE_TMPFS);
        return 0;
    }
    else if(strcmp(filesystem_type, "kernelfs") == 0)
    {
        mount_specifyFS(device, target, FS_TYPE_KERNELFS);
        return 0;
    }
    else
    {
        return -1;
    }
}

void init_mount()
{
    mount_list = list_create();
}