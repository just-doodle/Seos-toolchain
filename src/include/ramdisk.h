#ifndef __RAMDISK_H__
#define __RAMDISK_H__

#include "system.h"
#include "vfs.h"
#include "devfs.h"
#include "string.h"
#include "kheap.h"

typedef struct ramdisk_struct
{
    int num;
    uint32_t start;
    uint32_t end;
    size_t size;
    int is_writable;
}ramdisk_t;

void add_ramdisk(uint32_t start, uint32_t end, int isWritable);
void add_ramdisk_file(char* file, int isWritable);
int isRamdiskCreated(int idx);

#endif /*__RAMDISK_H__*/