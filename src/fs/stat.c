#include "stat.h"

int stat(char* name, seos_stat_t* stat)
{
    FILE* f = file_open(name, OPEN_RDONLY);
    if(f == NULL)
    {
        serialprintf("[STAT] File not found %s.\n", name);
        return -1;
    }
    uint32_t sz = vfs_getFileSize(f);
    stat->dev_id = f->dev_id;
    stat->ino = f->inode_num;
    stat->gid = f->gid;
    stat->uid = f->uid;
    stat->creation_time = f->creation_time;
    stat->last_accessed = f->last_accessed;
    stat->last_modified = f->last_modified;
    stat->mode = f->mask;
    stat->nlink = f->nlink;
    stat->blocks = (sz / 512);
    stat->blksize = 512;
    stat->size = (long)sz;
    serialprintf("STAT %s: ino: %d, devid: %d, size: %d:%d, blocks: %d, mode: %d\n", name, stat->ino, stat->dev_id, ((uint32_t)stat->size), sz, stat->blocks, stat->mode);
    return 0;
}