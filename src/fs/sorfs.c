#include "sorfs.h"
#include "timer.h"

void sorfs_readBlock(sorfs_t* fs, uint32_t blockNum, sorfs_block_t* block)
{
    vfs_read(fs->device, sizeof(sorfs_sb_t) + (blockNum * sizeof(sorfs_block_t)), sizeof(sorfs_block_t), (char*)block);

    if(block->magic != SORFS_BLOCK_MAGIC)
    {
        printf("[SORFS] Block %d is corrupted\n", blockNum);
        return;
    }
}

void sorfs_writeBlock(sorfs_t* fs, uint32_t blockIDX, sorfs_block_t* block)
{
    if(blockIDX > fs->total_blocks)
        return;

    if(block->magic != SORFS_BLOCK_MAGIC)
    {
        printf("[SORFS] Given block %d is corrupted\n", blockIDX);
        return;
    }

    vfs_write(fs->device, sizeof(sorfs_sb_t) + (blockIDX * sizeof(sorfs_block_t)), sizeof(sorfs_block_t), (char*)block);
}

void sorfs_readSB(sorfs_t* fs)
{
    vfs_read(fs->device, 0, sizeof(sorfs_sb_t), (char*)fs->sb);
}

void sorfs_rewrite_sb(sorfs_t* fs)
{
    vfs_write(fs->device, 0, sizeof(sorfs_sb_t), (char*)fs->sb);
}

void sorfs_readBlockContent(sorfs_t* fs, sorfs_block_t* block, uint32_t offset, uint32_t size, char *buffer)
{
    if(block != NULL)
        vfs_read(fs->device, block->offset + offset, size, buffer);
}

void sorfs_writeBlockContent(sorfs_t* fs, sorfs_block_t* block, uint32_t offset, uint32_t size, char* buffer)
{
    if(block == NULL)
        return;

    if((offset + size) > block->size)
    {
        uint32_t req_size = (offset+size) - block->size;
        if(sorfs_isLastBlock(fs, sorfs_blockToNum(fs, block)) == 1)
        {
            block->size += req_size;
            fs->sb->heap_left -= req_size;
        }
        else
        {
            serialprintf("[SORFS] Extended feature yet to be implemented!\n");
            printf("\033[31m[SORFS] Extended feature yet to be implemented!\033[m\n");
        }
    }

    if(block->offset == 0)
        return;

    vfs_write(fs->device, (block->offset + offset), size, buffer);
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

    if(block->type == SORFS_FRE)
        return NULL;

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
    sorfs_t* fs = (sorfs_t*)f->device;
    sorfs_block_t* block = zalloc(sizeof(sorfs_block_t));
    sorfs_readBlock(fs, f->inode_num, block);
    sorfs_writeBlockContent(fs, block, offset, size, buffer);
    sorfs_writeBlock(fs, f->inode_num, block);
    kfree(block);
    return size;
}

char** sorfs_listdir(FILE* node)
{
    sorfs_t* fs = (sorfs_t*)node->device;

    if(node == fs->sorfs_root)
    {
        char** list = kmalloc(sizeof(char*) * (fs->file_blocks) + 1);

        int i;
        for(i = 0; i < (fs->file_blocks); i++)
        {
            FILE* f = sorfs_BlockToVFSNode(fs, i);
            if(f == NULL)
                continue;
            list[i] = kmalloc(sizeof(char) * 64);
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
        for(int i = 0; i < (fs->file_blocks); i++)
        {
            sorfs_block_t* block = zalloc(sizeof(sorfs_block_t));
            sorfs_readBlock(fs, i, block);
            if(block->type == SORFS_FRE)
                continue;
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

sorfs_block_t* sorfs_allocBlock(sorfs_t* fs, int* idx)
{
    sorfs_block_t* block = ZALLOC_TYPES(sorfs_block_t);
    for(int i = 0; i < fs->total_blocks; i++)
    {
        sorfs_readBlock(fs, i, block);
        if(block->type == SORFS_FRE)
        {
            fs->sb->free_blocks--;
            block->type = SORFS_REG;
            *idx = i;
            sorfs_writeBlock(fs, i, block);
            return block;
        }
    }
}

uint32_t sorfs_allocFile(sorfs_t* fs, uint32_t size)
{
    sorfs_block_t* last_block = ZALLOC_TYPES(sorfs_block_t);
    sorfs_readBlock(fs, fs->file_blocks, last_block);
    if(memzero(last_block, sizeof(sorfs_block_t)) == 1)
        return -1;

    if(last_block->offset == 0)
        sorfs_readBlock(fs, fs->file_blocks-1, last_block);

    if(last_block->offset == 0)
        return -1;

    uint32_t __offset = (last_block->offset + last_block->size);
    free(last_block);
    return __offset;
}

int sorfs_isLastBlock(sorfs_t* fs, uint32_t blockIDX)
{
    sorfs_block_t* block = ZALLOC_TYPES(sorfs_block_t);
    sorfs_readBlock(fs, blockIDX, block);
    if(block->offset == 0 || block->magic != SORFS_BLOCK_MAGIC)
        return 0;

    if((blockIDX == fs->file_blocks || blockIDX == (fs->file_blocks-1)) && block->offset != 0 )
        return 1;
}

uint32_t sorfs_blockToNum(sorfs_t* fs, sorfs_block_t* block)
{
    sorfs_block_t* blk = ZALLOC_TYPES(sorfs_block_t);
    for(uint32_t i = 0; i < fs->file_blocks; i++)
    {
        sorfs_readBlock(fs, i, blk);
        if(memcmp(blk, block, sizeof(sorfs_block_t)) == 1)
        {
            free(blk);
            return i;
        }
    }
    return -1;
}

int sorfs_isLastBlockP(sorfs_t* fs, sorfs_block_t* a_block)
{
    sorfs_block_t* block = ZALLOC_TYPES(sorfs_block_t);
    sorfs_readBlock(fs, fs->file_blocks, block);
    if(block->magic != SORFS_BLOCK_MAGIC)
        return -1;

    if(block->offset == 0)
        sorfs_readBlock(fs, fs->file_blocks-1, block);

    if(block->offset == 0)
        return 0;

    if(memcmp(a_block, block, sizeof(sorfs_block_t)) == 0)
        return 1;
    else
        return 0;
}

void sorfs_createFile(FILE* root, char* name, uint32_t permissions)
{
    sorfs_t* fs = root->device;
    if(fs == NULL)
        return;

    if(fs->sorfs_root == root)
    {
        int blockidx = 0;
        sorfs_block_t* block = sorfs_allocBlock(fs, &blockidx);

        block->timestamp = gettimeofday_seconds();
        block->magic = SORFS_BLOCK_MAGIC;
        block->size = SORFS_DEFAULT_FILESIZE;
        block->flags = 0;
        block->offset = sorfs_allocFile(fs, block->size);
        strncpy(block->name, name, 64);
        fs->file_blocks++;
        sorfs_rewrite_sb(fs);
        if(blockidx != 0)
            sorfs_writeBlock(fs, blockidx, block);
        free(block);
        serialprintf("[SORFS] Created new file in block %d\n", blockidx);
    }   
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
    sorfs_block_t* blocks = kmalloc(sizeof(sorfs_block_t));
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
    serialprintf("Free Blocks: %d\n\n", fs->sb->free_blocks);
    serialprintf("Heap space left: %dMB\n", fs->sb->heap_left/MB);

    if(fs->sb->magic != SORFS_SUPERBLOCK_MAGIC)
    {
        printf("[SORFS] Not a valid SORFS filesystem\n");
        return 1;
    }

    fs->total_blocks = fs->sb->total_blocks;
    fs->file_blocks = fs->sb->total_blocks-fs->sb->free_blocks;

    printf("[SORFS] Mounting %s at %s\n", dev_path, mountpath);

    fs->sorfs_root = kmalloc(sizeof(vfs_node));
    memset(fs->sorfs_root, 0, sizeof(vfs_node));

    strcpy(fs->sorfs_root->name, mountpath);
    fs->sorfs_root->device = fs;
    fs->sorfs_root->listdir = sorfs_listdir;
    fs->sorfs_root->finddir = sorfs_finddir;
    fs->sorfs_root->readdir = sorfs_readdir;
    fs->sorfs_root->open = sorfs_open;
    fs->sorfs_root->close = sorfs_close;
    fs->sorfs_root->create = sorfs_createFile;
    fs->sorfs_root->size = fs->sb->total_blocks * sizeof(sorfs_block_t);

    fs->sorfs_root->flags = FS_DIRECTORY;

    vfs_mount(mountpath, fs->sorfs_root);

    sorfs_block_t* blo = ZALLOC_TYPES(sorfs_block_t);
    sorfs_readBlock(fs, 1, blo);
    uint32_t idx = sorfs_blockToNum(fs, blo);
    serialprintf("[SORFS] idx: %d || %d\n", idx, 3);
    ASSERT(idx == 1);

    printf("[SORFS] Mounted %s at %s\n", dev_path, mountpath);

    return 0;
}

DirectoryEntry* sorfs_readdir(FILE* node, uint32_t index)
{
    sorfs_t* fs = node->device;
    if(node == fs->sorfs_root)
    {
        sorfs_block_t* b = ZALLOC_TYPES(sorfs_block_t);
        sorfs_readBlock(fs, index, b);
        if(b->magic == SORFS_BLOCK_MAGIC)
        {
            DirectoryEntry* d = ZALLOC_TYPES(DirectoryEntry);
            d->inode_count = index;
            memcpy(d->name, b->name, 64);
            return d;
        }
        else
        {
            return NULL;
        }
    }
    return NULL;
}