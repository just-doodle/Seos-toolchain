#include "ext2.h"

void ext2_DebugPrintSB(ext2_fs_t* fs)
{
    serialprintf("Superblock:\n");
    serialprintf("Inodes: %d\n", fs->sb->total_inodes);
    serialprintf("Blocks: %d\n", fs->sb->total_blocks);
    serialprintf("Reserved blocks: %d\n", fs->sb->reserved_blocks);
    serialprintf("Free blocks: %d\n", fs->sb->free_blocks);
    serialprintf("Free inodes: %d\n", fs->sb->free_inodes);
    serialprintf("First data block: %d\n", fs->sb->first_block);
    serialprintf("block size: %d\n", (1024 << fs->sb->log_block_size));
    serialprintf("fragment size: %d\n", (1024 << fs->sb->log_frag_size));
    serialprintf("Blocks per group: %d\n", fs->sb->blocks_per_group);
    serialprintf("Fragments per group: %d\n", fs->sb->frags_per_group);
    serialprintf("Inodes per group: %d\n", fs->sb->inodes_per_group);
    serialprintf("Last mount time: %d\n", fs->sb->last_mnt_time);
    serialprintf("Last write time: %d\n", fs->sb->last_write_time);
    serialprintf("Mount count: %d\n", fs->sb->mount_num);
    serialprintf("Max mount count: %d\n", fs->sb->max_mount);
    serialprintf("Magic: %d [%s]\n", fs->sb->signature, (fs->sb->signature == EXT2_SIGNATURE) ? "OK" : "FAIL");
    serialprintf("State: %d [%s]\n", fs->sb->fs_state);
    serialprintf("err_catch: %d [%s]\n", fs->sb->err_catch, (fs->sb->err_catch == 0) ? "OK" : (fs->sb->err_catch == 1) ? "IGNORED" : (fs->sb->err_catch == 2) ? "REMOUNT RO" : "KERNEL PANIC");
    serialprintf("Last mount dir: %s\n", (char*)fs->sb->mnt_path);
}

void ext2_DebugPrintInode(ext2_inode_t* inode, ext2_fs_t* fs)
{
    serialprintf("Inode:\n");
    serialprintf("Mode: %d\n", inode->permission);
    serialprintf("UID: %d\n", inode->uid);
    serialprintf("GID: %d\n", inode->gid);
    serialprintf("Size: %d\n", inode->size_lo);
    serialprintf("num_sectors: %d\n", inode->sectors_used);
    serialprintf("Links: %d\n", inode->hard_links);
    serialprintf("Flags: %d\n\n", inode->flags);
}

int isext2(char* dev)
{
    ext2_fs_t* fs = ZALLOC_TYPES(ext2_fs_t);
    fs->block_size = EXT2_SUPERBLOCK_SIZE;
    fs->device = file_open(dev, OPEN_RDWR);

    fs->sb = ZALLOC_TYPES(ext2_sb_t);
    ext2_read_disk_block(fs, 1, fs->sb);

    ext2_DebugPrintSB(fs);

    if(fs->sb->signature != EXT2_SIGNATURE)
    {
        printf("[EXT2] The given device does not contain an ext2 filesystem.\n");
        free(fs->sb);
        vfs_close(fs->device);
        free(fs);
        return 0;
    }

    return 1;
}

int init_ext2(char* dev, char* mountpoint)
{
    ext2_fs_t* fs = ZALLOC_TYPES(ext2_fs_t);
    fs->block_size = EXT2_SUPERBLOCK_SIZE;
    fs->device = file_open(dev, OPEN_RDWR);

    fs->sb = ZALLOC_TYPES(ext2_sb_t);
    ext2_read_disk_block(fs, 1, fs->sb);

    if(fs->sb->signature != EXT2_SIGNATURE)
    {
        printf("[EXT2] The given device does not contain an ext2 filesystem.\n");
        free(fs->sb);
        vfs_close(fs->device);
        free(fs);
        return 1;
    }

    fs->blocks_per_group = fs->sb->blocks_per_group;
    fs->block_size = (1024 << fs->sb->log_block_size);
    fs->total_blocks = fs->sb->total_blocks;
    fs->total_groups = (fs->total_blocks / fs->blocks_per_group);

    if(fs->blocks_per_group * fs->total_groups < fs->total_groups)
        fs->total_groups++;

    fs->inode_size = fs->sb->inode_size;
    fs->inodes_per_group = fs->sb->inodes_per_group;

    fs->bgd_blocks = (fs->total_groups * sizeof(ext2_bgd_t)) / fs->block_size;

    if (fs->bgd_blocks * fs->block_size < fs->total_groups * sizeof(ext2_bgd_t))
        fs->bgd_blocks++;

    fs->bgd = zalloc(sizeof(ext2_bgd_t) * (fs->bgd_blocks*fs->block_size));

    for(int i = 0; i < fs->bgd_blocks; i++)
    {
        ext2_read_disk_block(fs, 2, (fs->bgd + i * fs->block_size));
    }

    fs->root_node = ZALLOC_TYPES(376);
    ext2_get_root(fs);
    vfs_mount(mountpoint, fs->root_node);

    return 0;
}

void ext2_read_inode_metadata(ext2_fs_t* fs, ext2_inode_t* inode, uint32_t inode_idx)
{
    uint32_t group = inode_idx / fs->inodes_per_group;
    uint32_t inode_table_block = fs->bgd[group].inode_table;
    uint32_t idx_in_group = inode_idx - group * fs->inodes_per_group;
    uint32_t block_offset = (idx_in_group - 1) * fs->sb->inode_size / fs->block_size;
    uint32_t offset_in_block = (idx_in_group - 1) - block_offset * (fs->block_size / fs->sb->inode_size);
    char *block_buf = zalloc(fs->block_size);
    serialprintf("{ext2} block_buf ptr: 0x%x : %dKB\n", block_buf, fs->block_size/KB);
    ext2_read_disk_block(fs, inode_table_block + block_offset, block_buf);
    memcpy(inode, block_buf + offset_in_block * fs->sb->inode_size, fs->sb->inode_size);
    serialprintf("{ext2} block_buf ptr: 0x%x\n", block_buf);
    ext2_DebugPrintInode(inode, fs);
    //kfree(block_buf);
}

void ext2_write_inode_metadata(ext2_fs_t* fs, ext2_inode_t* inode, uint32_t inode_idx)
{
    uint32_t group = inode_idx / fs->inodes_per_group;
    uint32_t inode_table_block = fs->bgd[group].inode_table;
    uint32_t block_offset = (inode_idx - 1) * fs->sb->inode_size / fs->block_size;
    uint32_t offset_in_block = (inode_idx - 1) - block_offset * (fs->block_size / fs->sb->inode_size);
    char *block_buf = (char *)kmalloc(fs->block_size);
    ext2_read_disk_block(fs, inode_table_block + block_offset, block_buf);
    memcpy(block_buf + offset_in_block * fs->sb->inode_size, inode, fs->sb->inode_size);
    ext2_write_disk_block(fs, inode_table_block + block_offset, block_buf);
    kfree(block_buf);
}

uint32_t ext2_read_inode_filedata(ext2_fs_t *fs, ext2_inode_t *inode, uint32_t offset, uint32_t size, char *buf)
{
    uint32_t end_offset = (inode->size_lo >= offset + size) ? (offset + size) : (inode->size_lo);
    uint32_t start_block = offset / fs->block_size;
    uint32_t end_block = end_offset / fs->block_size;
    uint32_t start_off = offset % fs->block_size;
    uint32_t end_size = end_offset - end_block * fs->block_size;

    uint32_t i = start_block;
    uint32_t curr_off = 0;
    while (i <= end_block)
    {
        uint32_t left = 0, right = fs->block_size - 1;
        char *block_buf = ext2_read_inode_block(fs, inode, i);
        if (i == start_block)
            left = start_off;
        if (i == end_block)
            right = end_size - 1;
        memcpy(buf + curr_off, block_buf + left, (right - left + 1));
        curr_off = curr_off + (right - left + 1);
        kfree(block_buf);
        i++;
    }
    return end_offset - offset;
}

char* ext2_read_inode_block(ext2_fs_t* fs, ext2_inode_t* inode, uint32_t inode_block)
{
    char *buf = (char *)kmalloc(fs->block_size);
    uint32_t direct_block_num = ext2_get_direct_block(fs, inode, inode_block);
    ext2_read_disk_block(fs, direct_block_num, buf);
    return buf;
}

void ext2_write_inode_block(ext2_fs_t* fs, ext2_inode_t* inode, uint32_t inode_block, char* buffer)
{
    uint32_t direct_block_num = ext2_get_direct_block(fs, inode, inode_block);
    ext2_write_disk_block(fs, direct_block_num, buffer);
}

void ext2_write_inode_filedata(ext2_fs_t *fs, ext2_inode_t *inode, uint32_t inode_idx, uint32_t offset, uint32_t size, char *buf)
{
    if (offset + size > inode->size_lo)
    {
        inode->size_lo = offset + size;
        ext2_write_inode_metadata(fs, inode, inode_idx);
    }
    uint32_t end_offset = (inode->size_lo >= offset + size) ? (offset + size) : (inode->size_lo);
    uint32_t start_block = offset / fs->block_size;
    uint32_t end_block = end_offset / fs->block_size;
    uint32_t start_off = offset % fs->block_size;
    uint32_t end_size = end_offset - end_block * fs->block_size;

    uint32_t i = start_block;
    uint32_t curr_off = 0;
    while (i <= end_block)
    {
        uint32_t left = 0, right = fs->block_size;
        char *block_buf = ext2_read_inode_block(fs, inode, i);

        if (i == start_block)
            left = start_off;
        if (i == end_block)
            right = end_size - 1;
        memcpy(block_buf + left, buf + curr_off, (right - left + 1));
        curr_off = curr_off + (right - left + 1);
        ext2_write_inode_block(fs, inode, i, block_buf);
        kfree(block_buf);
        i++;
    }
}

uint32_t ext2_get_direct_block(ext2_fs_t* fs, ext2_inode_t* inode, uint32_t inode_block)
{
    uint32_t p = fs->block_size / 4, res;
    int a, b, c, d, e, f, g, h;

    uint32_t *temp = (uint32_t *)kmalloc(fs->block_size);
    a = inode_block - EXT2_DIRECT_BLOCKS;

    if (a < 0)
    {
        res = inode->blocks[inode_block];
        goto exit;
    }

    b = a - p;
    if (b < 0)
    {
        ext2_read_disk_block(fs, inode->blocks[EXT2_DIRECT_BLOCKS], (char *)temp);
        res = temp[a];
        goto exit;
    }

    c = b - p * p;
    if (c < 0)
    {
        c = b / p;
        d = b - c * p;
        ext2_read_disk_block(fs, inode->blocks[EXT2_DIRECT_BLOCKS + 1], (char *)temp);
        ext2_read_disk_block(fs, temp[c], (char *)temp);
        res = temp[d];
        goto exit;
    }

    d = c - p * p * p;
    if (d < 0)
    {
        e = c / (p * p);
        f = (c - e * p * p) / p;
        g = (c - e * p * p - f * p);
        ext2_read_disk_block(fs, inode->blocks[EXT2_DIRECT_BLOCKS + 2], (char *)temp);
        ext2_read_disk_block(fs, temp[e], (char *)temp);
        ext2_read_disk_block(fs, temp[f], (char *)temp);
        res = temp[g];
        goto exit;
    }

exit:
    kfree(temp);
    return res;
}

void ext2_set_direct_block(ext2_fs_t* fs, ext2_inode_t* inode, uint32_t inode_idx, uint32_t inode_block, uint32_t disk_block)
{
    uint32_t p = fs->block_size / 4;
    int a, b, c, d, e, f, g;
    int iblock = inode_block;
    uint32_t *temp = (uint32_t *)kmalloc(fs->block_size);

    a = iblock - EXT2_DIRECT_BLOCKS;
    if (a <= 0)
    {
        inode->blocks[inode_block] = disk_block;
        goto exit;
    }

    b = a - p;
    if (b <= 0)
    {
        if (!ext2_allocate_inode_metadata_block(fs, inode, &(inode->blocks[EXT2_DIRECT_BLOCKS]), inode_idx, NULL, 0))
            ;
        ext2_read_disk_block(fs, inode->blocks[EXT2_DIRECT_BLOCKS], (char *)temp);
        ((uint32_t *)temp)[a] = disk_block;
        ext2_write_disk_block(fs, inode->blocks[EXT2_DIRECT_BLOCKS], (char *)temp);
        temp[a] = disk_block;
        goto exit;
    }

    c = b - p * p;
    if (c <= 0)
    {
        c = b / p;
        d = b - c * p;
        if (!ext2_allocate_inode_metadata_block(fs, inode, &(inode->blocks[EXT2_DIRECT_BLOCKS + 1]), inode_idx, NULL, 0))
            ;
        ext2_read_disk_block(fs, inode->blocks[EXT2_DIRECT_BLOCKS + 1], (char *)temp);
        if (!ext2_allocate_inode_metadata_block(fs, inode, &(temp[c]), inode_idx, (char *)temp, inode->blocks[EXT2_DIRECT_BLOCKS + 1]))
            ;
        uint32_t tmp = temp[c];
        ext2_read_disk_block(fs, tmp, (char *)temp);
        temp[d] = disk_block;
        ext2_write_disk_block(fs, tmp, (char *)temp);
        goto exit;
    }

    d = c - p * p * p;
    if (d <= 0)
    {
        e = c / (p * p);
        f = (c - e * p * p) / p;
        g = (c - e * p * p - f * p);
        if (!ext2_allocate_inode_metadata_block(fs, inode, &(inode->blocks[EXT2_DIRECT_BLOCKS + 2]), inode_idx, NULL, 0))
            ;
        ext2_read_disk_block(fs, inode->blocks[EXT2_DIRECT_BLOCKS + 2], (char *)temp);
        if (!ext2_allocate_inode_metadata_block(fs, inode, &(temp[e]), inode_idx, (char *)temp, inode->blocks[EXT2_DIRECT_BLOCKS + 2]))
            ;
        uint32_t tmp = temp[e];
        ext2_read_disk_block(fs, temp[e], (char *)temp);
        if (!ext2_allocate_inode_metadata_block(fs, inode, &(temp[f]), inode_idx, (char *)temp, tmp))
            ;
        tmp = temp[f];
        ext2_read_disk_block(fs, temp[f], (char *)temp);
        temp[g] = disk_block;
        ext2_write_disk_block(fs, tmp, (char *)temp);
        goto exit;
    }

exit:
    kfree(temp);
}

int ext2_allocate_inode_metadata_block(ext2_fs_t *fs, ext2_inode_t* inode, uint32_t *block_ptr, uint32_t inode_idx, char *buffer, uint32_t block_overwrite)
{
    if (!(*block_ptr))
    {
        uint32_t block_num = ext2_allocate_block(fs);
        if (!block_num)
            return 0;
        *block_ptr = block_num;
        if (buffer)
        {
            ext2_write_disk_block(fs, block_num, buffer);
        }
        else
        {
            ext2_write_inode_metadata(fs, inode, inode_idx);
        }
        return 1;
    }
    return 0;
}

uint32_t ext2_allocate_block(ext2_fs_t* fs)
{
    uint32_t *buf = (uint32_t *)zalloc(fs->block_size);
    for (uint32_t i = 0; i < fs->total_groups; i++)
    {
        if (!fs->bgd[i].free_blocks)
            continue;
        uint32_t bitmap_block = fs->bgd[i].block_bitmap;
        ext2_read_disk_block(fs, bitmap_block, (char *)buf);
        for (uint32_t j = 0; j < fs->block_size / 4; j++)
        {
            uint32_t sub_bitmap = buf[j];
            if (sub_bitmap == 0xFFFFFFFF)
                continue;
            for (uint32_t k = 0; k < 32; k++)
            {
                uint32_t free = !((sub_bitmap >> k) & 0x1);
                if (free)
                {
                    uint32_t mask = (0x1 << k);
                    buf[j] = buf[j] | mask;
                    ext2_write_disk_block(fs, bitmap_block, (char *)buf);
                    fs->bgd[i].free_blocks--;
                    ext2_rewrite_bgd(fs);
                    return i * fs->blocks_per_group + j * 32 + k;
                }
            }
        }
    }
    printf("[EXT2] Ran out of blocks for allocation!\n");
    return (uint32_t)-1;
}

void ext2_free_block(ext2_fs_t* fs, uint32_t block)
{
    uint32_t *buf = (uint32_t *)zalloc(fs->block_size);
    uint32_t group_num = block / fs->blocks_per_group;
    uint32_t sub_Bitmap_num = (block - (fs->blocks_per_group * group_num)) / 4;
    uint32_t index = (block - (fs->blocks_per_group * group_num)) % 4;

    uint32_t bitmap_block = fs->bgd[group_num].block_bitmap;
    ext2_read_disk_block(fs, bitmap_block, (char *)buf);

    uint32_t mask = ~(0x1 << index);
    buf[sub_Bitmap_num] = buf[sub_Bitmap_num] & mask;

    ext2_write_disk_block(fs, bitmap_block, (char *)buf);

    fs->bgd[group_num].free_blocks++;
    ext2_rewrite_bgd(fs);
}

void ext2_allocate_inode_block(ext2_fs_t *fs, ext2_inode_t *inode, uint32_t inode_idx, uint32_t block)
{
    uint32_t res = ext2_allocate_block(fs);
    ext2_set_direct_block(fs, inode, inode_idx, block, res);
    inode->sectors_used = (block + 1) * (fs->block_size / 512);

    ext2_write_inode_metadata(fs, inode, inode_idx);
}

void ext2_free_inode_block(ext2_fs_t *fs, ext2_inode_t *inode, uint32_t inode_idx, uint32_t block)
{
    uint32_t res = ext2_get_direct_block(fs, inode, block);
    ext2_free_block(fs, res);
    ext2_set_direct_block(fs, inode, inode_idx, res, 0);
    ext2_write_inode_metadata(fs, inode, inode_idx);
}

uint32_t ext2_allocate_inode(ext2_fs_t *fs)
{
    uint32_t *buf = (uint32_t *)zalloc(fs->block_size);

    for (uint32_t i = 0; i < fs->total_groups; i++)
    {
        if (!fs->bgd[i].free_inodes)
            continue;

        uint32_t bitmap_block = fs->bgd[i].inode_bitmap;
        ext2_read_disk_block(fs, bitmap_block, (char *)buf);
        for (uint32_t j = 0; j < fs->block_size / 4; j++)
        {
            uint32_t sub_bitmap = buf[j];
            if (sub_bitmap == 0xFFFFFFFF)
                continue;
            for (uint32_t k = 0; k < 32; k++)
            {
                uint32_t free = !((sub_bitmap >> k) & 0x1);
                if (free)
                {
                    uint32_t mask = (0x1 << k);
                    buf[j] = buf[j] | mask;
                    ext2_write_disk_block(fs, bitmap_block, (char *)buf);
                    fs->bgd[i].free_inodes--;
                    ext2_rewrite_bgd(fs);
                    return i * fs->inodes_per_group + j * 32 + k;
                }
            }
        }
    }
    printf("[EXT2] Ran out of inodes for allocation!\n");
    return (uint32_t)-1;
}

void ext2_rewrite_bgd(ext2_fs_t* fs)
{
    for (uint32_t i = 0; i < fs->bgd_blocks; i++)
        ext2_write_disk_block(fs, 2, (char *)fs->bgd + i * fs->block_size);
}

void ext2_rewrite_superblock(ext2_fs_t* fs)
{
    ext2_write_disk_block(fs, 1, (char *)fs->sb);
}

void ext2_free_inode(ext2_fs_t *fs, uint32_t inode)
{
    uint32_t *buf = (uint32_t *)kcalloc(fs->block_size, 1);
    uint32_t group_num = inode / fs->inodes_per_group;
    uint32_t sub_Bitmap_num = (inode - (fs->inodes_per_group * group_num)) / 4;
    uint32_t index = (inode - (fs->inodes_per_group * group_num)) % 4;

    uint32_t bitmap_block = fs->bgd[group_num].inode_bitmap;
    ext2_read_disk_block(fs, bitmap_block, (char *)buf);

    uint32_t mask = ~(0x1 << index);
    buf[sub_Bitmap_num] = buf[sub_Bitmap_num] & mask;

    ext2_write_disk_block(fs, bitmap_block, (char *)buf);

    fs->bgd[group_num].free_inodes++;
    ext2_rewrite_bgd(fs);
}

void ext2_read_disk_block(ext2_fs_t* fs, uint32_t block, char* buffer)
{
    FILE* dev = fs->device;
    vfs_read(dev, fs->block_size*block, fs->block_size, buffer);
}

void ext2_write_disk_block(ext2_fs_t* fs, uint32_t block, char* buffer)
{
    vfs_write(fs->device, fs->block_size*block, fs->block_size, buffer);
}

uint32_t ext2_getFileSize(FILE* node)
{
    ext2_inode_t* i = zalloc(sizeof(ext2_inode_t));
    ext2_read_inode_metadata(node->device, i, node->inode_num);
    size_t sz = i->size_lo;
    free(i);
    return sz;
}

void ext2_createEntry(FILE* node, char* name, uint32_t entry_inode)
{
    ext2_fs_t *fs = (ext2_fs_t*)node->device;
    ext2_inode_t *p_inode = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));
    ext2_read_inode_metadata(fs, p_inode, node->inode_num);
    uint32_t curr_offset = 0;
    uint32_t block_offset = 0;
    uint32_t in_block_offset = 0;
    uint32_t found = 0;
    uint32_t entry_name_len = strlen(name);
    char *check = (char*)zalloc(entry_name_len + 1);
    char *block_buf = ext2_read_inode_block(fs, p_inode, block_offset);

    while (curr_offset < p_inode->size_lo)
    {
        if (in_block_offset >= fs->block_size)
        {
            block_offset++;
            in_block_offset = 0;
            block_buf = ext2_read_inode_block(fs, p_inode, block_offset);
        }
        ext2_dirent_t *c_dir = (ext2_dirent_t*)(block_buf + in_block_offset);
        if (c_dir->name_len == entry_name_len)
        {
            memcpy(check, c_dir->name, entry_name_len);
            if (c_dir->inode != 0 && !strcmp(name, check))
            {
                printf("[EXT2] Entry by the same name %s already exist\n", check);
                return;
            }
        }

        if (found)
        {
            c_dir->inode = entry_inode;
            c_dir->total_size = (uint32_t)block_buf + fs->block_size - (uint32_t)c_dir;
            c_dir->name_len = strlen(name);
            c_dir->type = 0;

            memcpy(c_dir->name, name, strlen(name));
            ext2_write_inode_block(fs, p_inode, block_offset, block_buf);

            in_block_offset += c_dir->total_size;

            if (in_block_offset >= fs->block_size)
            {
                block_offset++;
                in_block_offset = 0;
                block_buf = ext2_read_inode_block(fs, p_inode, block_offset);
            }

            c_dir = (ext2_dirent_t *)(block_buf + in_block_offset);

            memset(c_dir, 0, sizeof(ext2_dirent_t));
            ext2_write_inode_block(fs, p_inode, block_offset, block_buf);

            return;
        }
        uint32_t expected_size = ((sizeof(ext2_dirent_t) + c_dir->name_len) & 0xfffffffc) + 0x4;
        uint32_t real_size = c_dir->total_size;
        if (real_size != expected_size)
        {
            found = 1;
            c_dir->total_size = expected_size;
            in_block_offset += expected_size;
            curr_offset += expected_size;
            continue;
        }
        in_block_offset += c_dir->total_size;
        curr_offset += c_dir->total_size;
    }
}

void ext2_removeEntry(FILE* node, char* name)
{
    ext2_fs_t *fs = (ext2_fs_t*)node->device;
    ext2_inode_t *p_inode = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));
    ext2_read_inode_metadata(fs, p_inode, node->inode_num);
    uint32_t curr_offset = 0;
    uint32_t block_offset = 0;
    uint32_t in_block_offset = 0;
    uint32_t entry_name_len = strlen(name);
    char *check = (char*)zalloc(entry_name_len + 1);
    char *block_buf = ext2_read_inode_block(fs, p_inode, block_offset);

    while (curr_offset < p_inode->size_lo)
    {
        if (in_block_offset >= fs->block_size)
        {
            block_offset++;
            in_block_offset = 0;
            block_buf = ext2_read_inode_block(fs, p_inode, block_offset);
        }
        ext2_dirent_t *c_dir = (ext2_dirent_t*)(block_buf + in_block_offset);
        if (c_dir->name_len == entry_name_len)
        {
            memcpy(check, c_dir->name, entry_name_len);
            if (c_dir->inode != 0 && !strcmp(name, check))
            {
                c_dir->inode = 0;
                ext2_write_inode_block(fs, p_inode, block_offset, block_buf);
                return;
            }
        }
        uint32_t expected_size = ((sizeof(ext2_dirent_t) + c_dir->name_len) & 0xfffffffc) + 0x4;
        uint32_t real_size = c_dir->total_size;

        if (real_size != expected_size)
            return;
        in_block_offset += c_dir->total_size;
        curr_offset += c_dir->total_size;
    }
}

void ext2_mkdir(FILE* node, char* name, uint16_t permission)
{
    ext2_fs_t* fs = (ext2_fs_t*)node->device;
    uint32_t inode_num = ext2_allocate_inode(fs);
    ext2_inode_t *inode = (ext2_inode_t *)kmalloc(sizeof(ext2_inode_t));
    ext2_read_inode_metadata(fs, inode, inode_num);
    inode->permission = EXT2_INODE_TYPE_DIR;
    inode->permission |= 0xFFF & permission;
    inode->atime = 0;
    inode->ctime = 0;
    inode->dtime = 0;
    inode->gid = 0;
    inode->uid = 0;
    inode->fragment_block_addr = 0;
    inode->sectors_used = 0;
    inode->size_lo = fs->block_size;
    inode->hard_links = 2;
    inode->flags = 0;
    inode->file_acl = 0;
    inode->dir_acl = 0;
    inode->generation = 0;
    inode->os_specific1 = 0;
    memset(inode->blocks, 0, sizeof(inode->blocks));
    memset(inode->os_specific2, 0, 12);

    ext2_allocate_inode_block(fs, inode, inode_num, 0);
    ext2_write_inode_metadata(fs, inode, inode_num);
    ext2_createEntry(node, name, inode_num);

    ext2_inode_t *p_inode = (ext2_inode_t *)kmalloc(sizeof(ext2_inode_t));
    ext2_read_inode_metadata(fs, p_inode, node->inode_num);
    p_inode->hard_links++;
    ext2_write_inode_metadata(fs, p_inode, node->inode_num);
    ext2_rewrite_bgd(fs);
}

void ext2_mkfile(FILE* node, char* name, uint16_t permission)
{
    ext2_fs_t *fs = (ext2_fs_t *)node->device;
    uint32_t inode_num = ext2_allocate_inode(fs);
    ext2_inode_t *inode = (ext2_inode_t *)kmalloc(sizeof(ext2_inode_t));
    ext2_read_inode_metadata(fs, inode, inode_num);
    inode->permission = EXT2_INODE_TYPE_REG;
    inode->permission |= 0xFFF & permission;
    inode->atime = 0;
    inode->ctime = 0;
    inode->dtime = 0;
    inode->gid = 0;
    inode->uid = 0;
    inode->fragment_block_addr = 0;
    inode->sectors_used = 0;
    inode->size_lo = fs->block_size;
    inode->hard_links = 2;
    inode->flags = 0;
    inode->file_acl = 0;
    inode->dir_acl = 0;
    inode->generation = 0;
    inode->os_specific1 = 0;
    memset(inode->blocks, 0, sizeof(inode->blocks));
    memset(inode->os_specific2, 0, 12);

    ext2_allocate_inode_block(fs, inode, inode_num, 0);
    ext2_write_inode_metadata(fs, inode, inode_num);
    ext2_createEntry(node, name, inode_num);

    ext2_inode_t *p_inode = (ext2_inode_t *)kmalloc(sizeof(ext2_inode_t));
    ext2_read_inode_metadata(fs, p_inode, node->inode_num);
    p_inode->hard_links++;
    ext2_write_inode_metadata(fs, p_inode, node->inode_num);
    ext2_rewrite_bgd(fs);
}

void ext2_unlink(FILE* node, char* name)
{
    ext2_fs_t *fs = (ext2_fs_t*)node->device;
    ext2_removeEntry(node, name);

    ext2_inode_t *p_inode = (ext2_inode_t *)kmalloc(sizeof(ext2_inode_t));
    ext2_read_inode_metadata(fs, p_inode, node->inode_num);
    p_inode->hard_links--;
    ext2_write_inode_metadata(fs, p_inode, node->inode_num);
    ext2_rewrite_bgd(fs);
}

char** ext2_listdir(FILE* node)
{
    ext2_fs_t * fs = node->device;
    ext2_inode_t * p_inode = kmalloc(sizeof(ext2_inode_t));
    ext2_read_inode_metadata(fs, p_inode, node->inode_num);
    uint32_t curr_offset = 0;
    uint32_t block_offset = 0;
    uint32_t in_block_offset = 0;
    int size = 0, cap = 10;
    char ** ret = zalloc(sizeof(char*) * cap);
    char * block_buf = ext2_read_inode_block(fs, p_inode, block_offset);
    while(curr_offset < p_inode->size_lo)
    {
        if(in_block_offset >= fs->block_size)
        {
            block_offset++;
            in_block_offset = 0;
            block_buf = ext2_read_inode_block(fs, p_inode, block_offset);
        }
        if(size + 1 == cap)
        {
            ret = krealloc(ret, sizeof(char*) * cap * 2);
            cap = cap * 2;
        }

        ext2_dirent_t * curr_dir = (ext2_dirent_t*)(block_buf + in_block_offset);
        if(curr_dir->inode != 0) {
            char * temp = zalloc(curr_dir->name_len + 1);
            memcpy(temp, curr_dir->name, curr_dir->name_len);
            ret[size++] = temp;
        }
        uint32_t expected_size = ((sizeof(ext2_dirent_t) + curr_dir->name_len) & 0xfffffffc) + 0x4;
        uint32_t real_size = curr_dir->total_size;
        if(real_size != expected_size)
        {
            break;
        }
        in_block_offset += curr_dir->total_size;
        curr_offset += curr_dir->total_size;
    }
    ret[size] = NULL;
    return ret;
}

FILE* ext2_finddir(FILE* node, char* name)
{
    ext2_fs_t *fs = (ext2_fs_t*)node->device;

    ext2_inode_t *p_inode = (ext2_inode_t *)kmalloc(sizeof(ext2_inode_t));
    ext2_read_inode_metadata(fs, p_inode, node->inode_num);

    uint32_t expected_size;
    uint32_t real_size;
    uint32_t curr_offset = 0;
    uint32_t block_offset = 0;
    uint32_t in_block_offset = 0;

    char *block_buf = ext2_read_inode_block(fs, p_inode, block_offset);

    while (curr_offset < p_inode->size_lo)
    {
        if (in_block_offset >= fs->block_size)
        {
            block_offset++;
            in_block_offset = 0;
            block_buf = ext2_read_inode_block(fs, p_inode, block_offset);
        }

        ext2_dirent_t *c_dir = (ext2_dirent_t *)(block_buf + in_block_offset);

        char *temp = (char *)kcalloc(c_dir->name_len + 1, 1);

        memcpy(temp, c_dir->name, c_dir->name_len);

        if (c_dir->inode != 0 && !strcmp(temp, name))
        {
            ext2_inode_t *inode = (ext2_inode_t *)kmalloc(sizeof(ext2_inode_t));
            ext2_read_inode_metadata(fs, inode, c_dir->inode);
            return ext2_dirent_to_node(fs, c_dir, inode);
        }

        if (((sizeof(ext2_dirent_t) + c_dir->name_len) & 0x00000003) != 0)
            expected_size = ((sizeof(ext2_dirent_t) + c_dir->name_len) & 0xfffffffc) + 0x4;
        else
            expected_size = ((sizeof(ext2_dirent_t) + c_dir->name_len) & 0xfffffffc);
        real_size = c_dir->total_size;

        if (real_size != expected_size)
        {
            break;
        }

        in_block_offset += c_dir->total_size;
        curr_offset += c_dir->total_size;
    }
    return NULL;
}

FILE* ext2_dirent_to_node(ext2_fs_t *fs, ext2_dirent_t *dir, ext2_inode_t *inode)
{
    FILE *res = (FILE*)zalloc(sizeof(FILE));

    res->device = (void *)fs;
    res->inode_num = dir->inode;
    memcpy(res->name, dir->name, dir->name_len);

    res->uid = inode->uid;
    res->uid = inode->gid;
    res->size = inode->size_lo;
    res->mask = inode->permission & 0xFFF;
    res->nlink = inode->hard_links;

    res->flags = 0;
    if ((inode->permission & EXT2_INODE_TYPE_REG) == EXT2_INODE_TYPE_REG)
    {
        res->flags |= FS_FILE;
        res->read = ext2_read;
        res->write = ext2_write;
        res->unlink = ext2_unlink;
        res->get_filesize = ext2_getFileSize;
    }
    if ((inode->permission & EXT2_INODE_TYPE_DIR) == EXT2_INODE_TYPE_DIR)
    {
        res->flags |= FS_DIRECTORY;
        res->mkdir = ext2_mkdir;
        res->finddir = ext2_finddir;
        res->unlink = ext2_unlink;
        res->create = ext2_mkfile;
        res->listdir = ext2_listdir;
        res->read = ext2_read;
        res->write = ext2_write;
    }
    if ((inode->permission & EXT2_INODE_TYPE_BLKDEV) == EXT2_INODE_TYPE_BLKDEV)
    {
        res->flags |= FS_BLOCKDEVICE;
    }
    if ((inode->permission & EXT2_INODE_TYPE_CHRDEV) == EXT2_INODE_TYPE_CHRDEV)
    {
        res->flags |= FS_CHARDEVICE;
    }
    if ((inode->permission & EXT2_INODE_TYPE_FIFO) == EXT2_INODE_TYPE_FIFO)
    {
        res->flags |= FS_PIPE;
    }
    if ((inode->permission & EXT2_INODE_TYPE_SYMLINK) == EXT2_INODE_TYPE_SYMLINK)
    {
        res->flags |= FS_SYMLINK;
    }

    res->last_accessed = inode->atime;
    res->last_modified = inode->mtime;
    res->creation_time = inode->ctime;

    res->chmod = ext2_chmod;
    res->open = ext2_open;
    res->close = ext2_close;
    return res;
}

void ext2_chmod(FILE* node, uint32_t mode)
{
    ext2_fs_t* fs = (ext2_fs_t*)node->device;
    ext2_inode_t *inode = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));
    ext2_read_inode_metadata(fs, inode, node->inode_num);
    inode->permission = (inode->permission & 0xFFFFF000) | mode;
    ext2_write_inode_metadata(fs, inode, node->inode_num);
}

uint32_t ext2_read(FILE *file, uint32_t offset, uint32_t size, char *buffer)
{
    ext2_fs_t *fs = (ext2_fs_t*)file->device;
    ext2_inode_t *inode = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));
    ext2_read_inode_metadata(fs, inode, file->inode_num);
    ext2_read_inode_filedata(fs, inode, offset, size, buffer);
    return size;
}

uint32_t ext2_write(FILE *file, uint32_t offset, uint32_t size, char *buffer)
{
    ext2_fs_t *fs = (ext2_fs_t*)file->device;
    ext2_inode_t *inode = (ext2_inode_t *)kmalloc(sizeof(ext2_inode_t));
    ext2_read_inode_metadata(fs, inode, file->inode_num);
    ext2_write_inode_filedata(fs, inode, file->inode_num, offset, size, buffer);
    return size;
}

void ext2_open(FILE *file, uint32_t flags)
{
    ext2_fs_t *fs = (ext2_fs_t*)file->device;

    if (flags & OPEN_TRUNC)
    {
        ext2_inode_t *inode = (ext2_inode_t*)kmalloc(sizeof(ext2_inode_t));
        ext2_read_inode_metadata(fs, inode, file->inode_num);
        inode->size_lo = 0;
        ext2_write_inode_metadata(fs, inode, file->inode_num);
        file->size = 0;
    }
}

void ext2_close(FILE* file)
{
    free(file);
    return;
}

void ext2_get_root(ext2_fs_t* fs)
{
    ext2_inode_t* inode = ZALLOC_TYPES(ext2_inode_t);
    ext2_read_inode_metadata(fs, inode, EXT2_ROOT_INODE);
    FILE* root = fs->root_node;
    strcpy(root->name, "ext2");
    root->device = fs;
    root->mask = inode->permission;
    root->inode_num = EXT2_ROOT_INODE;
    root->size = inode->size_lo;

    root->last_accessed = inode->atime;
    root->last_modified = inode->mtime;
    root->creation_time = inode->ctime;

    root->flags |= FS_DIRECTORY;

    root->read    = NULL;
    root->write   = NULL;
    root->chmod   = ext2_chmod;
    root->open    = ext2_open;
    root->close   = ext2_close;
    root->mkdir   = ext2_mkdir;
    root->create  = ext2_mkfile;
    root->listdir = ext2_listdir;
    root->finddir = ext2_finddir;
    root->unlink  = ext2_unlink;
    free(inode);
}