#ifndef __KERNELFS_H__
#define __KERNELFS_H__

#include "system.h"
#include "vfs.h"
#include "string.h"
#include "devfs.h"

#define KERNELFS_MAX_NODES 512

#define KERNELFS_TYPE_UNSIGNED_NUMBER 0
#define KERNELFS_TYPE_SIGNED_NUMBER 1
#define KERNELFS_TYPE_STRING 2
#define KERNELFS_TYPE_DATA_BUFFER 3

typedef struct kernelfs_struct
{
    char name[64];
    uint32_t size;
    void* ptr;
    int type;
}kernelfs_t;

void init_kernelfs();
void kernelfs_add_variable(char* name, void* ptr, uint32_t size, int type);

#endif /*__KERNELFS_H__*/