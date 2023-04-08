#include "kernelfs.h"

kernelfs_t kfs_nodes[KERNELFS_MAX_NODES];
int c_node = 0;
FILE* kernelfs_root;



uint32_t kernelfs_read(FILE* f, uint32_t offset, size_t size, char* buffer)
{
    kernelfs_t* node = f->device;

    switch(node->type)
    {
    case KERNELFS_TYPE_UNSIGNED_NUMBER:
    case KERNELFS_TYPE_SIGNED_NUMBER:
    case KERNELFS_TYPE_DATA_BUFFER:
        {
            memcpy(buffer, node->ptr, node->size);
        }break;
    case KERNELFS_TYPE_STRING:
        {
            strcpy(buffer, node->ptr);
        }break;
    default:
        memcpy(buffer, node->ptr, node->size);
        break;
    };

    return node->type;
}

uint32_t kernelfs_write(FILE* f, uint32_t offset, size_t size, char* buffer)
{
    kernelfs_t* n = f->device;
    strcpy(buffer, "Cannot write to a kernelfs file\n");
    return n->type;
}

uint32_t kernelfs_getFileSize(FILE* f)
{
    kernelfs_t* n = f->device;
    return n->size;
}

void kernelfs_open(FILE* f, uint32_t flags)
{
    return;
}

void kernelfs_close(FILE* f)
{
    return;
}

FILE* kernelfsNode_to_FILE(kernelfs_t* node)
{
    FILE* f = ZALLOC_TYPES(FILE);
    strcpy(f->name, node->name);
    f->inode_num = node->type;
    f->size = node->size;
    f->device = node;
    f->flags = FS_CHARDEVICE;
    f->open = kernelfs_open;
    f->close = kernelfs_close;
    f->read = kernelfs_read;
    f->write = kernelfs_write;
    f->get_filesize = kernelfs_getFileSize;
    return f;
}

char** kernelfs_listdir(FILE* p_node)
{
    int i = 0;
    if(p_node == kernelfs_root)
    {
        char** list = calloc(sizeof(char), c_node);
        for(i = 0; i < c_node; i++)
        {
            list[i] = zalloc(64);
            strcpy(list[i], kfs_nodes[i].name);
        }
        list[i++] = NULL;
        return list;
    }
    return NULL;
}

FILE* kernelfs_finddir(FILE* p_node, char* file)
{
    if(p_node == kernelfs_root)
    {
        for(int i = 0; i < c_node; i++)
        {
            if(strcmp(file, kfs_nodes[i].name) == 0)
            {
                return kernelfsNode_to_FILE(&kfs_nodes[i]);
            }
        }
        return NULL;
    }
    return NULL;
}

void init_kernelfs()
{
    kernelfs_root = ZALLOC_TYPES(FILE);
    c_node = 0;
    memset(kfs_nodes, 0, sizeof(kernelfs_t)*KERNELFS_MAX_NODES);
    strcpy(kernelfs_root->name, "kernel");
    kernelfs_root->flags = FS_DIRECTORY;
    kernelfs_root->finddir = kernelfs_finddir;
    kernelfs_root->listdir = kernelfs_listdir;
    kernelfs_root->open = kernelfs_open;
    kernelfs_root->close = kernelfs_close;
    devfs_add(kernelfs_root);
}

void kernelfs_add_variable(char* name, void* ptr, uint32_t size, int type)
{
    kernelfs_t* node = &kfs_nodes[c_node++];
    strncpy(node->name, name, 64);
    node->ptr = ptr;
    node->size = size;
    node->type = type;
}