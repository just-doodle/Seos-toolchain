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

#define SORFS_REG 	0
#define SORFS_LNK	 1
#define SORFS_DIRP	2
#define SORFS_DIRC	3

#define ERR_DNF 1
#define ERR_IOR  2
#define ERR_SME 3

typedef struct sorfs_superblock
{
	uint32_t magic;
	char name[32];
	uint32_t total_blocks;
	uint32_t block_size;
}s_superblock_t;

typedef struct sorfs_block
{
	uint32_t magic;
	char name[64];
	uint32_t type;
	uint32_t parent_block;
	uint32_t size;
	uint32_t timestamp;
	uint32_t offset;
}s_block_t;

int create_sorfs(char** files, char* output);
int decode_sorfs(char* path, char* output);

#endif
