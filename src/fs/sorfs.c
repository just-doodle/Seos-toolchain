#include "sorfs.h"


void sorfs_readBlock(sorfs_t* fs, uint32_t blockNum, sorfs_block_t* block)
{
    vfs_read(fs->device, sizeof(sorfs_sb_t) + (blockNum * sizeof(sorfs_block_t)), sizeof(sorfs_block_t), (char*)block);

    if(block->magic != SORFS_BLOCK_MAGIC)
    {
        printf("[SORFS] Block %d is corrupted\n", blockNum);
        return;
    }
}

void sorfs_readSB(sorfs_t* fs)
{
    vfs_read(fs->device, 0, sizeof(sorfs_sb_t), (char*)fs->sb);
}

void sorfs_readBlockContent(sorfs_t* fs, sorfs_block_t* block, uint32_t offset, uint32_t size, char *buffer)
{
    vfs_read(fs->device, block->offset + offset, size, buffer);
}

uint32_t sorfs_getFileSize(FILE* f)
{
    sorfs_t* fs = (sorfs_t*)f->device;
    sorfs_block_t* block = kmalloc(sizeof(sorfs_block_t));
    sorfs_readBlock(fs, f->inode_num, block);
    uint32_t size = block->size;
    kfree(block);
    return size;
}

FILE* sorfs_BlockToVFSNode(sorfs_t* fs, uint32_t blockNum)
{
    if(blockNum >= fs->sb->total_blocks)
    {
        printf("[SORFS] Block %d is out of bounds\n", blockNum);
        return NULL;
    }

    FILE* f = kmalloc(sizeof(FILE));
    sorfs_block_t* block = kmalloc(sizeof(sorfs_block_t));
    sorfs_readBlock(fs, blockNum, block);

    strcpy(f->name, block->name);
    f->device = fs;
    f->open = sorfs_open;
    f->close = sorfs_close;
    f->size = block->size;
    f->inode_num = blockNum;
    f->creation_time = block->timestamp;
    f->dev_id = fs->device->inode_num;
    f->fs_type = FS_TYPE_SORFS;

    if(block->type == SORFS_DIRP)
    {
        f->mask = _IFDIR;
        f->finddir = sorfs_finddir;
    }

    if(block->type == SORFS_REG)
    {
        f->mask = _IFREG;
        f->read = sorfs_read;
        f->write = sorfs_write;
        f->get_filesize = sorfs_getFileSize;
    }
    
    return f;
}

uint32_t sorfs_read(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    sorfs_t* fs = (sorfs_t*)f->device;
    sorfs_block_t* block = zalloc(sizeof(sorfs_block_t));
    sorfs_readBlock(fs, f->inode_num, block);
    sorfs_readBlockContent(fs, block, offset, size, buffer);
    kfree(block);
    return size;
}

uint32_t sorfs_write(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    printf("[SORFS] Cannot write to a read-only filesystem\n");
    return -1;
}

char** sorfs_listdir(FILE* node)
{
    sorfs_t* fs = (sorfs_t*)node->device;

    if(node == fs->sorfs_root)
    {
        char** list = kmalloc(sizeof(char*) * (fs->sb->total_blocks) + 1);

        int i;
        for(i = 0; i < (fs->total_blocks); i++)
        {
            FILE* f = sorfs_BlockToVFSNode(fs, i);
            if(f == NULL)
                continue;
            list[i] = kmalloc(sizeof(char) * 256);
            strcpy(list[i], f->name);
        }
        list[i] = NULL;
        return list;
    }
    return NULL;
}

FILE* sorfs_finddir(FILE* node, char* name)
{
    sorfs_t* fs = (sorfs_t*)node->device;
    if(node == fs->sorfs_root)
    {
        for(int i = 0; i < (fs->total_blocks); i++)
        {
            sorfs_block_t* block = zalloc(sizeof(sorfs_block_t));
            sorfs_readBlock(fs, i, block);
            FILE* f = sorfs_BlockToVFSNode(fs, i);
            if(f == NULL)
                continue;
            if(strcmp(block->name, name) == 0)
                return f;
        }
        return NULL;
    }
    return NULL;
}

void sorfs_open(FILE* f, uint32_t flags)
{
    f->open_flags = flags;
    return;
}

void sorfs_close(FILE* f)
{
    free(f);
    return;
}

int isSORFS(char* devpath)
{
    sorfs_t* fs = (sorfs_t*)kmalloc(sizeof(sorfs_t));
    fs->device = file_open(devpath, 0);
    fs->sb = kmalloc(sizeof(sorfs_sb_t));
    sorfs_readSB(fs);
    if(fs->sb->magic != SORFS_SUPERBLOCK_MAGIC)
    {
        vfs_close(fs->device);
        kfree(fs);
        return 0;
    }
    vfs_close(fs->device);
    return 1;
}

void debug_sorfs(sorfs_t* fs)
{
    sorfs_block_t* blocks = kmalloc(fs->sb->block_size);
    for(int i = 0; i < fs->sb->total_blocks; i++)
    {
        sorfs_readBlock(fs, i, blocks);
        serialprintf("SORFS_Block #%d\n", i);
        serialprintf("\t-Name: %s\n", blocks->name);
        serialprintf("\t-Magic: 0x%06x [%s]\n", blocks->magic, (blocks->magic == SORFS_BLOCK_MAGIC ? "OK" : "CORRUPTED"));
        serialprintf("\t-Offset: 0x%06x\n", blocks->offset);
        serialprintf("\t-Type: %s\n", (blocks->type == SORFS_REG ? "Regular file" : (blocks->type == SORFS_LNK ? "Link to file" : "Unknown")));
        serialprintf("\t-Size: %dB\n", blocks->size);
    }
}

int init_sorfs(char* dev_path, char* mountpath)
{
    sorfs_t* fs = kmalloc(sizeof(sorfs_t));
    fs->device = file_open(dev_path, OPEN_RDWR);
    fs->sb = kmalloc(sizeof(sorfs_sb_t));
    sorfs_readSB(fs);

    serialprintf("\nSORFS_SB:\n");
    serialprintf("Name: %s\n", fs->sb->name);
    serialprintf("Magic: 0x%06x [%s]\n", fs->sb->magic, (fs->sb->magic == SORFS_SUPERBLOCK_MAGIC ? "OK" : "CORRUPTED"));
    serialprintf("Total Blocks: %d\n\n", fs->sb->total_blocks);

    if(fs->sb->magic != SORFS_SUPERBLOCK_MAGIC)
    {
        printf("[SORFS] Not a valid SORFS filesystem\n");
        return 1;
    }

    fs->total_blocks = fs->sb->total_blocks;

    printf("[SORFS] Mounting %s at %s\n", dev_path, mountpath);

    fs->sorfs_root = kmalloc(sizeof(vfs_node));
    memset(fs->sorfs_root, 0, sizeof(vfs_node));

    strcpy(fs->sorfs_root->name, mountpath);
    fs->sorfs_root->device = fs;
    fs->sorfs_root->listdir = sorfs_listdir;
    fs->sorfs_root->finddir = sorfs_finddir;
    fs->sorfs_root->open = sorfs_open;
    fs->sorfs_root->close = sorfs_close;
    fs->sorfs_root->size = fs->sb->total_blocks * fs->sb->block_size;

    fs->sorfs_root->flags = FS_DIRECTORY;

    vfs_mount(mountpath, fs->sorfs_root);

    debug_sorfs(fs);

    printf("[SORFS] Mounted %s at %s\n", dev_path, mountpath);

    return 0;
}