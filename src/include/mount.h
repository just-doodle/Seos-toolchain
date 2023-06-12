#ifndef __MOUNT_H__
#define __MOUNT_H__

#include "system.h"
#include "printf.h"
#include "string.h"
#include "list.h"

typedef struct mount_info_struct
{
    uint32_t fs_type;
    char* device;
    char* mountpoint;
}mount_info_t;

extern list_t* mount_list;

int mount(char* devpath, char* mountpoint);
void init_mount();
void print_mountList();

int syscall_mount(const char* device, const char* target, const char* filesystem_type, int mount_flags, char* fsflags);

#endif /*__MOUNT_H__*/