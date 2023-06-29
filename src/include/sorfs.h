#ifndef __SORFS_H__
#define __SORFS_H__

#include "system.h"
#include "kheap.h"
#include "string.h"
#include "vfs.h"
#define SORFS_SUPERBLOCK_MAGIC 0xb252e80d
#define SORFS_BLOCK_MAGIC 0x528fd70c

#define SORFS_DEFAULT_FILESIZE 1

#define SORFS_FRE 0
#define SORFS_REG 1
#define SORFS_LNK 2
#define SORFS_DIR 3

#define SORFS_FLAGS_USE_EXTBLOCKS (1 << 0)

#define SORFS_TOTAL_BLOCKS 256
#define SORFS_TOTAL_EXTBLOCKS 256
#define SORFS_TOTAL_HEAP_SPACE 25*MB

// SORFS blockmap
// ____________________
// |---Superblock*1---| : 44B       Contains information about the partition
// |---FileBlocks*256-| : 88*256B   Contains information about files
// |---ExtBlocks*256--| : 12*256B   Used in case the file written has size larger than current size
// |---FileData-------| : -------   Used by file blocks for storing data
// |---FileHeap*25MB--| : 25MB      Used when creating new files 
// |__________________|
// FileBlocks*256 + ExtBlocks*256 = 25KB

typedef struct sorfs_superblock
{
    uint32_t magic;             // Superblock magic
    char name[32];              // Name of the volume
    uint32_t total_blocks;      // Total blocks in the filesystem
	uint32_t free_blocks;       // Number of free blocks in the filesystem to be used for making new files
    uint32_t total_extblocks;   // Total ExtBlocks in filesystems (ExtBlocks are the blocks which are used for saving extra data on old files)
	uint32_t free_extblocks;    // Free ExtBlocks in filesystem to be used for extending the file
    uint32_t heap_left;         // Space left in the file heap
}sorfs_sb_t;

typedef struct sorfs_blocks
{
    uint32_t magic;     // Magic for the block
    char name[64];      // Name of file
    uint32_t type;      // File type
    uint32_t size;      // size of file (if the block is directory, then number of files in directory)
    uint32_t timestamp; // Timestamp of created file
    uint32_t offset;    // Offset of file in disk
    uint8_t flags;      // Flags related to block
}sorfs_block_t;

typedef struct sorfs_extblock_struct
{
    uint32_t parent_block;      // The index to parent block
    uint32_t size;              // Size of extension
    uint32_t offset;            // Offset of the start of extension
}sorfs_extblock_t;

typedef struct sorfs
{
    FILE* device;

    sorfs_sb_t *sb;
    uint32_t total_blocks;
    uint32_t file_blocks;
    vfs_node* sorfs_root;
} sorfs_t;

int sorfs_test(char* dev);
int sorfs_mount(char* dev, char* mountpoint);

void init_sorfs();

uint32_t sorfs_read(FILE* f, uint32_t offset, uint32_t size, char* buffer);
uint32_t sorfs_write(FILE* f, uint32_t offset, uint32_t size, char* buffer);

void sorfs_open(FILE* f, uint32_t offset);
void sorfs_close(FILE* f);

char** sorfs_listdir(FILE* node);
FILE* sorfs_finddir(FILE* node, char* name);
DirectoryEntry* sorfs_readdir(FILE* node, uint32_t index);

void sorfs_createFile(FILE* root, char* name, uint32_t permissions);

void sorfs_writeBlockContent(sorfs_t* fs, sorfs_block_t* block, uint32_t offset, uint32_t size, char* buffer);

uint32_t sorfs_blockToNum(sorfs_t* fs, sorfs_block_t* block);

sorfs_block_t* sorfs_allocBlock(sorfs_t* fs, int* idx);
uint32_t sorfs_allocFile(sorfs_t* fs, uint32_t size);
int sorfs_isLastBlock(sorfs_t* fs, uint32_t blockIDX);
int sorfs_isLastBlockP(sorfs_t* fs, sorfs_block_t* a_block);

#endif /*__SORFS_H__*/