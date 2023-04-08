#include "mount.h"

#include "sorfs.h"
#include "vfs.h"

int mount(char* devpath, char* mountpoint)
{
    uint32_t t = find_fs(devpath);
    if(t == FS_TYPE_SORFS)
    {
        init_sorfs(devpath, mountpoint);
        return 0;
    }
    else
    {
        return -1;
    }
}