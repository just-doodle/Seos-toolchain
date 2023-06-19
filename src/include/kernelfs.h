#ifndef __KERNELFS_H__
#define __KERNELFS_H__

#include "system.h"
#include "vfs.h"
#include "string.h"
#include "devfs.h"

typedef struct kernelfs_node_struct
{
    char name[32];
    void* ptr;
    uint32_t len;
    listnode_t* self;
}kernelfs_node_t;

typedef struct kernelfs_struct
{
    list_t* nodes;
    FILE* root;
}kernelfs_t;

void init_kernelfs(char* mountpoint);
void kernelfs_add_variable(const char* root, char* name, void* ptr, uint32_t size);

#endif /*__KERNELFS_H__*/