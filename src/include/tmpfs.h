#ifndef __TMPFS_H__
#define __TMPFS_H__

#include "system.h"
#include "kheap.h"
#include "vfs.h"
#include "string.h"
#include "printf.h"

#define TMPFS_MAGIC 0xDEADABCD

#define TMPFS_MAX_FILES 512

typedef struct tmpfs_file_struct
{
    uint32_t idx;
    uint32_t magic;
    char* buffer;
    uint32_t current_size;
    int type;
    FILE* self;
}tmpfs_file_t;

typedef struct tmpfs_struct
{
    uint32_t n_files;
    uint32_t size;

    tmpfs_file_t* bitmap;
}tmpfs_t;

void init_tmpfs(char* mountpoint);

uint32_t tmpfs_read(FILE* f, uint32_t offset, uint32_t size, char* buffer);
uint32_t tmpfs_write(FILE* f, uint32_t offset, uint32_t size, char* buffer);

char** tmpfs_listdir(FILE* parent);
FILE* tmpfs_finddir(FILE* parent, char* name);

uint32_t tmpfs_getfilesize(FILE* f);

void tmpfs_open(FILE* f, uint32_t mode);
void tmpfs_close(FILE* f);

void tmpfs_create(FILE* parent, char* name, uint32_t permission);

DirectoryEntry* tmpfs_readdir(FILE* node, uint32_t index);

#endif /*__TMPFS_H__*/