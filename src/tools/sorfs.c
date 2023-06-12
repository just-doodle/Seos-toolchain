#include "sorfs.h"
#include "math.h"
#include <fcntl.h>
#include <sys/stat.h>

#define MAXLIST 100

uint32_t seek = 0;

int parseSpace(char *str, char **parsed)
{
    int i;

    for (i = 0; i < MAXLIST; i++)
    {
        parsed[i] = strsep(&str, " ");

        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
    return i;
}

int getstr(char* str)
{
	int j = 0;
	while((str[j] = getchar()) != '\n')
	{
		j++;
	}
	
	str[j] = '\0';
	return j;
}

uint32_t getfilectime(char *path)
{
	struct stat attr;
	stat(path, &attr);
	return attr.st_ctime;
}

uint32_t get_size(FILE * f)
{
	fseek(f, 0L, SEEK_END);
	uint32_t sz = ftell(f);
	fseek(f, 0L, SEEK_SET);
	return sz;
}

int pad(char* path)
{
	FILE* in = fopen(path, "rb+");
	uint32_t size = get_size(in);
	if(size % 512 != 0)
	{
		uint32_t p = size % 512;
		uint32_t m = (size - p)/512;
		uint32_t pd = (m+1)*512;
		uint32_t ppd = pd - size;
		fseek(in, 0, SEEK_END);
		char* b = malloc(ppd);
		memset(b, 0, ppd);
		fwrite(b, ppd, 1, in);
		fclose(in);
		free(b);
		return 0;
	}
	return 1;
}

// int create_sorfs(char **files, char *out)
// {
// 	printf("Creating new sorfs image...\n");
// 	FILE *of = fopen(out, "wb");
// 	uint32_t count = 0;
// 	for (count = 0; files[count] != NULL; count++);
// 	sorfs_sb_t superblock;
// 	superblock.magic = SORFS_SUPERBLOCK_MAGIC;
// 	strcpy(superblock.name, basename(out));
// 	superblock.total_blocks = count;
// 	superblock.block_size = sizeof(s_block_t);
// 	fwrite((void *)&superblock, sizeof(sorfs_sb_t), 1, of);

// 	uint32_t ufoffset = sizeof(sorfs_sb_t) + count * superblock.block_size;
// 	uint32_t uboffset = sizeof(sorfs_sb_t);

// 	char *fb = NULL;

// 	s_block_t *blk = malloc(sizeof(s_block_t));

// 	uint32_t cpos = 0;

// 	for (int i = 0; i < count; i++)
// 	{
// 		if (fb != NULL)
// 			free(fb);
// 		char *fn = files[i];
// 		FILE *in = fopen(fn, "rb");
// 		if (in == NULL)
// 		{
// 			printf("%s file not found!\nExiting...\n", fn);
// 			return 1;
// 		}

// 		memset(blk, 0, sizeof(s_block_t));
// 		uint32_t size = get_size(in);
// 		blk->magic = SORFS_BLOCK_MAGIC;
// 		strcpy(blk->name, basename(fn));
// 		blk->parent_block = 0;	// Resides in root directory / superblock
// 		blk->type = SORFS_REG;
// 		blk->offset = ufoffset;
// 		blk->size = size;
// 		blk->timestamp = getfilectime(fn);

// 		fseek(of, uboffset, SEEK_SET);
// 		fwrite((void *)blk, sizeof(s_block_t), 1, of);

// 		uboffset += sizeof(s_block_t);

// 		fseek(of, ufoffset, SEEK_SET);
// 		fb = malloc(size);
// 		fread(fb, size, 1, in);
// 		fwrite(fb, size, 1, of);

// 		fclose(in);

// 		ufoffset += size;
// 	}

// 	fclose(of);
// 	free(fb);
// 	free(blk);

// 	pad(out);
	
// 	printf("Created new sorfs image %s\n", out);

// }

int create_sorfs(char **files, char *out)
{
	uint32_t count = 0;
	for (count = 0; files[count] != NULL; count++);
	uint32_t __sz = (sizeof(sorfs_sb_t) + (sizeof(sorfs_block_t) * 256) + (sizeof(sorfs_extblock_t) * 256));
	for(int i = 0; i < count; i++)
	{
		FILE* f = fopen(files[i], "rb");
		__sz += get_size(f);
		fclose(f);
	}

	__sz += (25 * (1024 * 1024));

	uint32_t __offset = 0;

	char* buf = malloc(__sz);
	memset(buf, 0, __sz);

	sorfs_sb_t* sb = (sorfs_sb_t*)(buf);
	sb->heap_left = (25 * (1024 * 1024));
	sb->total_extblocks = sb->free_extblocks = 256;
	sb->total_blocks = 256;
	sb->free_blocks = (sb->total_blocks - count);
	sb->magic = SORFS_SUPERBLOCK_MAGIC;
	strcpy(sb->name, basename(out));

	__offset += sizeof(sorfs_sb_t);

	sorfs_block_t* blocks = (sorfs_block_t*)(buf+__offset);

	for(int i = 0; i < sb->total_blocks; i++)
	{
		blocks[i].flags = 0;
		blocks[i].type = SORFS_FRE;
		blocks[i].magic = SORFS_BLOCK_MAGIC;
	}

	__offset += (sizeof(sorfs_block_t) * sb->total_blocks);

	sorfs_extblock_t *extblocks = (sorfs_extblock_t*)(buf+__offset);

	for(int i = 0; i < sb->total_extblocks; i++)
	{
		extblocks[i].offset = 0;
		extblocks[i].parent_block = 0;
		extblocks[i].size = 0;
	}

	__offset += (sizeof(sorfs_extblock_t) * sb->total_extblocks);

	char* fileData = (buf + __offset);

	uint32_t __foffset = __offset;
	uint32_t __freloffset = 0;

	for(int i = 0; i < count; i++)
	{
		FILE* f = fopen(files[i], "rb");
		if(f == NULL)
		{
			continue;
		}
		blocks[i].magic = SORFS_BLOCK_MAGIC;
		blocks[i].type = SORFS_REG;
		blocks[i].size = get_size(f);
		blocks[i].timestamp = getfilectime(files[i]);
		strcpy(blocks[i].name, basename(files[i]));
		blocks[i].offset = __foffset;
		fread((fileData+__freloffset), blocks[i].size, 1, f);
		__freloffset += blocks[i].size;
		__foffset += blocks[i].size;
		fclose(f);
	}

	printf("[SORFS] Created. Stats:\nout_name = %s\nout_size = %dB\nfiledata_size = %dB\ntotal_files: %dB\nfree_blocks = %d\n", out, __sz, __freloffset, sb->total_blocks, sb->free_blocks);
	FILE* ob = fopen(out, "wb");
	fwrite(buf, __sz, 1, ob);
	fclose(ob);
	free(buf);

	printf("[SORFS] Padding\n");
	pad(out);
}

int decode_sorfs(char *path, char *out)
{
	sorfs_sb_t sb;
	FILE *in = fopen(path, "rb");
	FILE *of = NULL;
	fread((void *)&sb, sizeof(sorfs_sb_t), 1, in);
	if (sb.magic != SORFS_SUPERBLOCK_MAGIC)
	{
		fclose(in);
		return ERR_SME;
	}

	printf("Superblock:\n\t%s\n\ttotal blocks: %d\n\tfree blocks: %d\n\theap left: %dB\n\ttotal extblocks: %d\n\tfree extblocks: %d\n", sb.name, sb.total_blocks, sb.free_blocks,
		   sb.heap_left, sb.total_extblocks, sb.free_extblocks);

	if (chdir(out) != 0)
	{
		mkdir(out, 0777);
		chdir(out);
	}

	sorfs_block_t b[sb.total_blocks];
	fread((void *)b, (sizeof(sorfs_block_t) * sb.total_blocks), 1, in);
	char *fb = NULL;

	char* h = malloc(256);

	for (int i = 0; i < sb.total_blocks; i++)
	{
		if (b[i].magic != SORFS_BLOCK_MAGIC)
		{
			printf("Block %d is invalid. Skipping...\n", i);
			continue;
		}

		if(b[i].type == SORFS_FRE)
			continue;

		printf
			("block %d:\n\t%s\n\tsize:%dB\n\ttype:%d\n\toffset: 0x%06x\n\ttimestamp: %d\n\tparent block: %d\n",
			 i, b[i].name, b[i].size, b[i].type, b[i].offset, b[i].timestamp, b[i].flags);


		if (fb != NULL)
			free(fb);

		of = fopen(b[i].name, "wb");
		fseek(in, b[i].offset, SEEK_SET);
		fb = malloc(b[i].size);
		fread(fb, b[i].size, 1, in);
		fwrite(fb, b[i].size, 1, of);
		fclose(of);

		struct timespec taccess[2];
		taccess[0].tv_nsec = b[i].timestamp;
		taccess[0].tv_nsec = 0;
		taccess[1].tv_nsec = 0;
		taccess[1].tv_nsec = 0;

		memset(h, 0, 256);
		sprintf(h, "%s/%s", out, b[i].name);
		utimensat(0, h, taccess, 0);

		printf("Extracted %s...\n", b[i].name);
	}

	fclose(in);
}

int main(int argc, char **argv)
{
	if(argc < 4)
	{
		printf("Usage: \n%s -c output files...\n%s -e outdir image\n", argv[0], argv[0]);
		return 1;	
	}
	
	if(strcmp(argv[1], "-c") == 0)
	{
		char* output = argv[2];
		char* files[argc - 3];
		int i = 0;
		for(i = 0; i < (argc - 3); i++)
		{
			files[i] = argv[i + 3];	
		}
		
		files[i] = NULL;
		
		create_sorfs(files, output);
	}
	else if(strcmp(argv[1], "-e") == 0)
	{
		char* outdir = argv[2];
		char* img = argv[3];
		
		decode_sorfs(img, outdir);	
	}
	
	return 0;
}
