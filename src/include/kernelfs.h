#ifndef __KERNELFS_H__
#define __KERNELFS_H__

#include "system.h"
#include "vfs.h"
#include "string.h"
#include "devfs.h"

#define KERNELFS_FLAGS_CAN_FREE (1<<0)

typedef struct kernelfs_node_struct
{
    char name[32];
    void* ptr;
    uint32_t len;
    uint32_t flags;
    listnode_t* self;
}kernelfs_node_t;

typedef struct kernelfs_struct
{
    list_t* nodes;
    FILE* root;
}kernelfs_t;

void init_kernelfs();
void kernelfs_add_variable(const char* root, char* name, void* ptr, uint32_t size);
void kernelfs_addcharf(const char* root, char* name, char* fmt, ...);

#endif /*__KERNELFS_H__*/