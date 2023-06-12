#ifndef __SORFS__
#define __SORFS__

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "sys/stat.h"
#include "sys/types.h"
#include <libgen.h>
#include "stdint.h"

#define SORFS_SUPERBLOCK_MAGIC 0xb252e80d
#define SORFS_BLOCK_MAGIC 0x528fd70c

#define ERR_DNF 1
#define ERR_IOR  2
#define ERR_SME 3

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
    uint32_t magic;
    char name[32];
    uint32_t total_blocks;
	uint32_t free_blocks;
    uint32_t total_extblocks;
	uint32_t free_extblocks;
    uint32_t heap_left;
}sorfs_sb_t;

typedef struct sorfs_blocks
{
    uint32_t magic;
    char name[64];
    uint32_t type;
    uint32_t size;
    uint32_t timestamp;
    uint32_t offset;
    uint8_t flags;
}sorfs_block_t;

typedef struct sorfs_extblock_struct
{
    uint32_t parent_block;
    uint32_t size;
    uint32_t offset;
}sorfs_extblock_t;

int create_sorfs(char** files, char* output);
int decode_sorfs(char* path, char* output);

#endif
