#ifndef __FAT32_H__
#define __FAT32_H__

#include "system.h"
#include "vfs.h"
#include "kheap.h"
#include "vfs.h"
#include "logdisk.h"

typedef struct fat32_bpb_struct
{
    uint8_t jmp[3];
    char oem[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t numFATs;
    uint16_t numRootDirEnt;
    uint16_t numLogicalSectors;
    uint8_t mediaType;
    uint16_t numSectorsPerFAT;
    uint16_t numSectorsPerTrack;
    uint16_t totalHeads;
    uint32_t startLBA;
    uint32_t totalSectorCount;

    uint32_t FATSize;
    uint16_t extflags;
    uint16_t FATVersion;
    uint32_t rootCluster;
    uint16_t FSInfo;
    uint16_t BackupBootSector;
    uint8_t reserved[12];
    uint8_t driveIDX;
    uint8_t NTFlags;
    uint8_t signature;
    uint32_t volumeID;
    char volLabel[11];
    char sysID[8];
    uint8_t bootCode[420];
    uint16_t bootSignature;
}__attribute__(( packed )) fat32_bpb_t;

typedef struct fat32_fsinfo_struct
{
    uint32_t lead_signature;
    uint8_t reserved[420];
    uint32_t signature;
    uint32_t freeCluster;
    uint32_t firstCluster;
    uint8_t reserved2[12];
    uint32_t trail_signature;
}__attribute__(( packed ))fat32_fsinfo_t;

typedef struct fat32_dirent_struct
{
    char file[8];
    char ext[3];
    uint8_t attributes;
    uint8_t NTReserved;
    uint8_t ctimeTenth;
    uint16_t ctime;
    uint16_t cdate;
    uint16_t adate;
    uint16_t firstClusterHi;
    uint16_t mtime;
    uint16_t mdate;
    uint16_t firstClusterLo;
    uint32_t size;
}__attribute__(( packed ))fat32_dirent_t;

typedef struct fat32_struct
{
    fat32_bpb_t* bpb;

    uint32_t FATStart;
    uint32_t FATSize;

    uint32_t dataStart;
    uint32_t rootStart;

    uint32_t rootFiles;

    FILE* device;
}fat32_t;

void init_fat32(char* device);

#endif /*__FAT32_H__*/