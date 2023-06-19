#include "kernelfs.h"
#include "mount.h"
#include "debug.h"
#include "timer.h"


kernelfs_node_t* kernelfs_get_node_from_file(FILE* f)
{
    kernelfs_t* fs = f->device;
    kernelfs_node_t* n = NULL;
    foreach(l, fs->nodes)
    {
        n = l->val;
        if(strcmp(n->name, f->name) == 0)
        {
            return n;
        }
    }

    return NULL;
}

uint32_t kernelfs_read(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    if(validate(buffer) != 1)
        return -1;

    if(validate(f) != 1)
        return -1;

    kernelfs_t* fs = f->device;
    kernelfs_node_t* n = kernelfs_get_node_from_file(f);
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

    kernelfs_t* fs = f->device;

    if(validate(fs->nodes) != 1)
        return NULL;
    char** list = zalloc(sizeof(uintptr_t) * (list_size(fs->nodes)+1));
    int k = 0;
    foreach(l, fs->nodes)
    {
        kernelfs_node_t* n = l->val;
        list[k] = strndup(n->name, 32);
        k++;
    }
    list[k] = NULL;
    return list;
}

uint32_t get_nodeNUM(kernelfs_t* fs, kernelfs_node_t* n)
{
    if(validate(n) != 1)
        return -1;

    if(validate(fs) != 1)
        return -1;

    uint32_t j = 0;

    foreach(l, fs->nodes)
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
    kernelfs_t* fs = f->device;
    kernelfs_node_t* n = kernelfs_get_node_from_file(f);

    return n->len;
}

FILE* kernelfs_to_FILE(kernelfs_t* fs, kernelfs_node_t* n)
{
    if(validate(n) != 1)
        return NULL;

    FILE* f = ZALLOC_TYPES(FILE);
    if(validate(f) != 1)
        return NULL;

    strcpy(f->name, n->name);
    f->size = n->len;
    f->device = fs;
    uint32_t inode = get_nodeNUM(fs, n);
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
    if((validate(f) != 1) || (validate(name) != 1))
    {
        return NULL;
    }

    kernelfs_t* fs = f->device;
    if(validate(fs) != 1)
        return NULL;

    foreach(l, fs->nodes)
    {
        kernelfs_node_t* n = l->val;
        if(strcmp(name, n->name) == 0)
            return kernelfs_to_FILE(fs, n);
    }
}

DirectoryEntry* kernelfs_readdir(FILE* f, uint32_t idx)
{
    if((validate(f) != 1) || (validate(f->device) != 1))
        return NULL;

    kernelfs_t* fs = f->device;

    if(idx > list_size(fs->nodes))
        return NULL;

    kernelfs_node_t* n = list_get_node_by_index(fs->nodes, idx)->val;
    DirectoryEntry* d = ZALLOC_TYPES(DirectoryEntry);
    strncpy(d->name, n->name, 32);
    d->inode_count = get_nodeNUM(fs, n);
    return d;
}

void init_kernelfs(char* mountpoint)
{
    kernelfs_t* fs = ZALLOC_TYPES(kernelfs_t);
    fs->nodes = list_create();
    fs->root = ZALLOC_TYPES(FILE);
    strcpy(fs->root->name, "kernelfs");
    fs->root->device = fs;
    fs->root->flags = FS_DIRECTORY;
    fs->root->fs_type = FS_TYPE_KERNELFS;
    fs->root->open = kernelfs_open;
    fs->root->close = kernelfs_close;
    fs->root->listdir = kernelfs_listdir;
    fs->root->finddir = kernelfs_finddir;
    fs->root->readdir = kernelfs_readdir;
    if(validate(mount_list) == 1)
    {
        mount_info_t* m = ZALLOC_TYPES(mount_info_t);
        m->device = "kernel";
        m->mountpoint = strdup(mountpoint);
        m->fs_type = FS_TYPE_KERNELFS;
        list_push(mount_list, m);
    }
    vfs_mount(mountpoint, fs->root);
}

void kernelfs_add_variable(const char* root, char* name, void* ptr, uint32_t size)
{
    FILE* f = file_open(root, 0);
    if(validate(f) != 1)
        return;
    kernelfs_t* fs = f->device;
    if(validate(fs->nodes) != 1)
        return NULL;

    kernelfs_node_t* n = ZALLOC_TYPES(kernelfs_node_t);
    strncpy(n->name, name, 32);
    n->ptr = ptr;
    n->len = size;
    n->self = list_insert_front(fs->nodes, n);
    serialprintf("variable %s with size %d added to %s\n", name, size, root);
}