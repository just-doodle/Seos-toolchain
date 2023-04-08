#include "sorfs.h"

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
	
	str[j] = "\0";
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

int create_sorfs(char **files, char *out)
{
	printf("Creating new sorfs image...\n");
	FILE *of = fopen(out, "wb");
	uint32_t count = 0;
	for (count = 0; files[count] != NULL; count++);
	s_superblock_t superblock;
	superblock.magic = SORFS_SUPERBLOCK_MAGIC;
	strcpy(superblock.name, basename(out));
	superblock.total_blocks = count;
	superblock.block_size = sizeof(s_block_t);
	fwrite((void *)&superblock, sizeof(s_superblock_t), 1, of);

	uint32_t ufoffset = sizeof(s_superblock_t) + count * superblock.block_size;
	uint32_t uboffset = sizeof(s_superblock_t);

	char *fb = NULL;

	s_block_t *blk = malloc(sizeof(s_block_t));

	uint32_t cpos = 0;

	for (int i = 0; i < count; i++)
	{
		if (fb != NULL)
			free(fb);
		char *fn = files[i];
		FILE *in = fopen(fn, "rb");
		if (in == NULL)
		{
			printf("%s file not found!\nExiting...\n", fn);
			return 1;
		}

		memset(blk, 0, sizeof(s_block_t));
		uint32_t size = get_size(in);
		blk->magic = SORFS_BLOCK_MAGIC;
		strcpy(blk->name, basename(fn));
		blk->parent_block = 0;	// Resides in root directory / superblock
		blk->type = SORFS_REG;
		blk->offset = ufoffset;
		blk->size = size;
		blk->timestamp = getfilectime(fn);

		fseek(of, uboffset, SEEK_SET);
		fwrite((void *)blk, sizeof(s_block_t), 1, of);

		uboffset += sizeof(s_block_t);

		fseek(of, ufoffset, SEEK_SET);
		fb = malloc(size);
		fread(fb, size, 1, in);
		fwrite(fb, size, 1, of);

		fclose(in);

		ufoffset += size;
	}

	fclose(of);
	free(fb);
	free(blk);
	
	printf("Created new sorfs image %s\n", out);

}

int decode_sorfs(char *path, char *out)
{
	s_superblock_t sb;
	FILE *in = fopen(path, "rb");
	FILE *of = NULL;
	fread((void *)&sb, sizeof(s_superblock_t), 1, in);
	if (sb.magic != SORFS_SUPERBLOCK_MAGIC)
	{
		fclose(in);
		return ERR_SME;
	}

	printf("Superblock:\n\t%s\n\ttotal blocks: %d\n\tblock size: %dB\n", sb.name, sb.total_blocks,
		   sb.block_size);

	if (chdir(out) != 0)
	{
		mkdir(out, 0777);
		chdir(out);
	}

	s_block_t b[sb.total_blocks];
	fread((void *)b, (sizeof(s_block_t) * sb.total_blocks), 1, in);
	char *fb = NULL;

	for (int i = 0; i < sb.total_blocks; i++)
	{
		if (b[i].magic != SORFS_BLOCK_MAGIC)
		{
			printf("Block %d is invalid. Skipping...\n", i);
			continue;
		}
		printf
			("block %d:\n\t%s\n\tsize:%dB\n\ttype:%d\n\toffset: 0x%06x\n\ttimestamp: %d\n\tparent block: %d\n",
			 i, b[i].name, b[i].size, b[i].type, b[i].offset, b[i].timestamp, b[i].parent_block);


		if (fb != NULL)
			free(fb);

		of = fopen(b[i].name, "wb");
		fseek(in, b[i].offset, SEEK_SET);
		fb = malloc(b[i].size);
		fread(fb, b[i].size, 1, in);
		fwrite(fb, b[i].size, 1, of);
		fclose(of);

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
