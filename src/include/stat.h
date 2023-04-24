#ifndef __STAT_H__
#define __STAT_H__

#include "system.h"
#include "vfs.h"
#include "kheap.h"
#include "printf.h"
#include "string.h"

typedef struct stat_struct
{
    uint64_t dev_id;
    unsigned long ino;
    uint32_t mode;
    uint32_t nlink;
    uint32_t uid;
    uint32_t gid;
    long size;

    uint32_t last_modified;
    uint32_t last_accessed;
    uint32_t creation_time;
    long blocks;
    long int blksize;
}seos_stat_t;

int stat(char* name, seos_stat_t* stat);

#endif /*__STAT_H__*/