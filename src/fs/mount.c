#include "mount.h"
#include "sorfs.h"
#include "vfs.h"
#include "tmpfs.h"
#include "process.h"
#include "ext2.h"
#include "kernelfs.h"

list_t* mount_list = NULL;

int mount(char* device, char* mountpoint)
{
    uint32_t fsuid = find_fs(device);
    vfs_fsinfo_t* fs = get_fs_by_uid(fsuid);
    if(validate(fs) != 1)
        return -1;

    if(!fs->mount)
        return -1;

    if(!(fs->flags & FSINFO_FLAGS_CUSTOM_MOUNT_ENTRY))
    {
        mount_info_t* m = ZALLOC_TYPES(mount_info_t);
        m->device = strdup(device);
        m->mountpoint = strdup(mountpoint);
        m->fs_type = fs->uid;
        list_push(mount_list, m);
    }

    int ret = fs->mount(device, mountpoint);
    return ret;
}

int syscall_mount(const char* device, const char* target, const char* filesystem_type, int mount_flags, char* fsflags)
{
    vfs_fsinfo_t* fs = get_fs_by_name(filesystem_type);
    if(validate(fs) != 1)
        return -1;

    if((!(fs->flags & FSINFO_FLAGS_NODEV)) && !(fs->test(device)))
        return -1;

    if(!(fs->flags & FSINFO_FLAGS_CUSTOM_MOUNT_ENTRY))
    {
        mount_info_t* m = ZALLOC_TYPES(mount_info_t);
        m->device = strdup(device);
        m->mountpoint = strdup(target);
        m->fs_type = fs->uid;
        list_push(mount_list, m);
    }
    
    int ret = fs->mount(device, target);
    return ret;

}

void print_mountList()
{
    foreach(l, mount_list)
    {
        mount_info_t* m = l->val;
        printf("%s -> %s [%s]\n", m->device, m->mountpoint, get_fs_by_uid(m->fs_type)->name);
    }
}

void init_mount()
{
    mount_list = list_create();
}