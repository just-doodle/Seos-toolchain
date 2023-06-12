#include "kernelfs.h"
#include "mount.h"
#include "debug.h"
#include "timer.h"

list_t* kernelfs_nodes = NULL;
FILE* kernelfs_root = NULL;

uint32_t kernelfs_read(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    if(validate(buffer) != 1)
        return -1;

    if(validate(f) != 1)
        return -1;


    kernelfs_node_t* n = f->device;
    if(validate(n) != 1)
        return -1;
    if(validate(n->ptr) != 1)
        return -1;
    if((offset+size) > n->len)
        return -1;

    memcpy(buffer, n->ptr+offset, size);
    return size;
}

char** kernelfs_listdir(FILE* f)
{
    if(validate(f) != 1)
        return NULL;
    if(validate(kernelfs_nodes) != 1)
        return NULL;

    if(f == kernelfs_root)
    {
        char** list = zalloc(sizeof(uintptr_t) * (list_size(kernelfs_nodes)+1));
        int k = 0;
        foreach(l, kernelfs_nodes)
        {
            kernelfs_node_t* n = l->val;
            list[k] = strndup(n->name, 32);
            k++;
        }
        list[k] = NULL;
        return list;
    }
}

uint32_t get_nodeNUM(kernelfs_node_t* n)
{
    if(validate(n) != 1)
        return -1;
    if(validate(kernelfs_nodes) != 1)
        return -1;

    uint32_t j = 0;

    foreach(l, kernelfs_nodes)
    {
        if(memcmp(n, l->val, sizeof(kernelfs_node_t)) == 1)
        {
            return j;
        }
        j++;
    }
}

void kernelfs_open(FILE* f, uint32_t flags)
{
    return;
}

void kernelfs_close(FILE* f)
{
    return;
}

uint32_t kernelfs_getFileSize(FILE* f)
{
    if(validate(f) != 1)
        return 0;
    kernelfs_node_t* n = f->device;
    if(validate(n) != 1)
        return 0;

    return n->len;
}

FILE* kernelfs_to_FILE(kernelfs_node_t* n)
{
    if(validate(n) != 1)
        return NULL;

    FILE* f = ZALLOC_TYPES(FILE);
    if(validate(f) != 1)
        return NULL;

    strcpy(f->name, n->name);
    f->size = n->len;
    f->device = n;
    uint32_t inode = get_nodeNUM(n);
    if(inode != -1)
        f->inode_num = inode;

    f->open = kernelfs_open;
    f->close = kernelfs_close;
    f->read = kernelfs_read;
    f->write = 0;
    f->get_filesize = kernelfs_getFileSize;
    f->flags = FS_BLOCKDEVICE;
    f->last_accessed = gettimeofday_seconds();
    return f;
}

FILE* kernelfs_finddir(FILE* f, char* name)
{
    if((validate(f) != 1) || (validate(name) != 1) || (validate(kernelfs_nodes) != 1))
    {
        return NULL;
    }

    foreach(l, kernelfs_nodes)
    {
        kernelfs_node_t* n = l->val;
        if(validate(n) != 1)
            continue;
        if(strcmp(name, n->name) == 0)
            return kernelfs_to_FILE(n);
    }
}

DirectoryEntry* kernelfs_readdir(FILE* f, uint32_t idx)
{
    if((validate(f) != 1) || (validate(f->device) != 1))
        return NULL;

    if(idx > list_size(kernelfs_nodes))
        return NULL;

    kernelfs_node_t* n = list_get_node_by_index(kernelfs_nodes, idx)->val;
    if(validate(n) != 1)
        return NULL;
    DirectoryEntry* d = ZALLOC_TYPES(DirectoryEntry);
    strncpy(d->name, n->name, 32);
    d->inode_count = get_nodeNUM(n);
    return d;
}

void init_kernelfs(char* mountpoint)
{
    kernelfs_nodes = list_create();
    kernelfs_root = ZALLOC_TYPES(FILE);
    strcpy(kernelfs_root->name, "kernelfs");
    kernelfs_root->flags = FS_DIRECTORY;
    kernelfs_root->fs_type = FS_TYPE_KERNELFS;
    kernelfs_root->open = kernelfs_open;
    kernelfs_root->close = kernelfs_close;
    kernelfs_root->listdir = kernelfs_listdir;
    kernelfs_root->finddir = kernelfs_finddir;
    kernelfs_root->readdir = kernelfs_readdir;
    if(validate(mount_list) == 1)
    {
        mount_info_t* m = ZALLOC_TYPES(mount_info_t);
        m->device = "kernel";
        m->mountpoint = strdup(mountpoint);
        m->fs_type = FS_TYPE_KERNELFS;
        list_push(mount_list, m);
    }
    vfs_mount(mountpoint, kernelfs_root);
}

void kernelfs_add_variable(char* name, void* ptr, uint32_t size)
{
    if((validate(kernelfs_root) != 1) || (validate(kernelfs_nodes) != 1) || (validate(name) != 1) || (validate(ptr) != 1))
        return;

    kernelfs_node_t* n = ZALLOC_TYPES(kernelfs_node_t);
    strncpy(n->name, name, 32);
    n->ptr = ptr;
    n->len = size;
    n->self = list_insert_front(kernelfs_nodes, n);
}