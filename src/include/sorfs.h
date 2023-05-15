#ifndef __SORFS_H__
#define __SORFS_H__

#include "system.h"
#include "kheap.h"
#include "string.h"
#include "vfs.h"

#define SORFS_SUPERBLOCK_MAGIC 0xb252e80d
#define SORFS_BLOCK_MAGIC 0x528fd70c

#define SORFS_REG 0
#define SORFS_LNK 1
#define SORFS_DIRP 2
#define SORFS_DIRC 3

typedef struct sorfs_superblock
{
    uint32_t magic;
    char name[32];
    uint32_t total_blocks;
    uint32_t block_size;
}sorfs_sb_t;

typedef struct sorfs_blocks
{
    uint32_t magic;
    char name[64];
    uint32_t type;
    uint32_t parent_block;
    uint32_t size;
    uint32_t timestamp;
    uint32_t offset;
}sorfs_block_t;


typedef struct sorfs
{
    FILE* device;

    sorfs_sb_t *sb;
    uint32_t total_blocks;
    vfs_node* sorfs_root;
} sorfs_t;

int isSORFS(char* dev);
int init_sorfs(char* dev, char* mountpoint);

uint32_t sorfs_read(FILE* f, uint32_t offset, uint32_t size, char* buffer);
uint32_t sorfs_write(FILE* f, uint32_t offset, uint32_t size, char* buffer);

void sorfs_open(FILE* f, uint32_t offset);
void sorfs_close(FILE* f);

char** sorfs_listdir(FILE* node);
FILE* sorfs_finddir(FILE* node, char* name);
DirectoryEntry* sorfs_readdir(FILE* node, uint32_t index);

#endif /*__SORFS_H__*/