#ifndef __MBR_H__
#define __MBR_H__

#include "system.h"
#include "string.h"
#include "devfs.h"
#include "vfs.h"
#include "kheap.h"

#define MBR_MAGIC 0xAA55

typedef struct PartitionTableEntry
{
    uint8_t bootable;
    uint8_t start_head;
    uint8_t start_sector : 6;
    uint16_t start_cylinder : 10;
    uint8_t partition_id;
    uint8_t end_head;
    uint8_t end_sector : 6;
    uint16_t end_cylinder : 10;
    
    uint32_t start_lba;
    uint32_t length;
} __attribute__((packed)) pte_t;

typedef struct mbr_struct
{
    uint8_t bootloader[440];
    uint32_t signature;
    uint16_t unused;
    
    pte_t primaryPartition[4];
    
    uint16_t magicnumber;
} __attribute__((packed)) mbr_t;

typedef struct partition_struct
{
    FILE* device;

    int partition_num;
    uint8_t bootable;

    uint8_t partition_id;

    uint32_t start;
    uint32_t size;

    uint32_t fs_type;
}partition_t;

int probe_partitions(char* dev_name);

#endif /*__MBR_H__*/