#include "mbr.h"

partition_t partitions[4];
FILE partitions_node[4];

uint32_t mbr_read(FILE* node, uint32_t offset, uint32_t size, char* buffer)
{
    partition_t* partition = node->device;
    
    uint32_t toff = (partition->start * 512) + offset;

    if(offset + size > (partition->size * 512))
    {
        printf("[MBR] Attempted to read past end of partition\n");
        return -1;
    }

    return vfs_read(partition->device, toff, size, buffer);
}

uint32_t mbr_write(FILE* node, uint32_t offset, uint32_t size, char* buffer)
{
    partition_t* partition = node->device;
    
    uint32_t toff = partition->start + offset;
    if(offset + size > partition->size*512)
    {
        printf("[MBR] Attempted to write past end of partition\n");
        return -1;
    }

    return vfs_write(partition->device, toff, size, buffer);
}

void mbr_open(FILE* node, uint32_t flags)
{
    // Nothing to do
}

void mbr_close(FILE* node)
{
    // Nothing to do
}


FILE* mbr_getVFSNode(partition_t* partition)
{
    FILE* node = zalloc(sizeof(FILE));
    strcpy(node->name, partition->device->name);
    strcat(node->name, "p");
    strcat(node->name, itoa_r(partition->partition_num, 10));

    node->size = partition->size * 512;
    node->flags = FS_BLOCKDEVICE;
    node->device = partition;
    node->read = mbr_read;
    node->write = mbr_write;
    node->open = mbr_open;
    node->close = mbr_close;

    partitions_node[partition->partition_num] = *node;

    return node;
}

int probe_partitions(char* device)
{
    printf("[MBR] Probing partitions on %s\n", device);
    FILE* f = file_open(device, 0);
    if(f == NULL)
    {
        printf("[MBR] Could not open %s\n", device);
        return 1;
    }
    
    mbr_t* mbr = kmalloc(512);
    vfs_read(f, 0, 512, (char*)mbr);
    vfs_close(f);

    if(mbr->magicnumber != 0xAA55)
    {
        printf("[MBR] Invalid MBR\n");
        return 1;
    }

    char* partPath = kcalloc(sizeof(char) * (strlen("/dev/") + 20), 1);
    pte_t partition;

    for(int i = 0; i < 4 && mbr->primaryPartition[i].partition_id != 0x00; i++)
    {
        memset(partPath, 0, sizeof(char) * (strlen("/dev/") + 20));
        partition = mbr->primaryPartition[i];
        partitions[i].bootable = partition.bootable;
        partitions[i].device = f;
        partitions[i].partition_num = i;
        partitions[i].partition_id = partition.partition_id;
        partitions[i].start = partition.start_lba;
        partitions[i].size = partition.length;
        devfs_add(mbr_getVFSNode(&partitions[i]));
        strcpy(partPath, "/dev/");
        strcat(partPath, partitions_node[i].name);
        partitions[i].fs_type = find_fs(partPath);
    }

    return 0;
}