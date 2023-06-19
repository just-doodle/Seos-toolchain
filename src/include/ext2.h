#ifndef __EXT2_H__
#define __EXT2_H__

#include "system.h"
#include "vfs.h"
#include "kheap.h"
#include "math.h"

#define EXT2_SUPERBLOCK_SIZE 1024

#define EXT2_FS_STATE_CLEAN 0x01
#define EXT2_FS_STATE_ERROR 0x02

#define EXT2_ERROR_IGNORE 0x01
#define EXT2_ERROR_REMOUNT_RO 0x02
#define EXT2_ERROR_KPANIC 0x03

#define EXT2_OSID_LINUX 0x00
#define EXT2_OSID_HURD 0x01
#define EXT2_OSID_MASIX 0x03
#define EXt2_OSID_FREEBSD 0x04
#define EXT2_OSID_LITES 0x05

#define EXT2_OPTIONAL_PREALLOCATE_BLOCK 0x0001
#define EXT2_OPTIONAL_AFS_INODES_EXIST 0x0002
#define EXT2_OPTIONAL_HAS_JOURNAL 0x0004
#define EXT2_OPTIONAL_INODE_EXTENDED_ATTRIBUTES 0x0008
#define EXT2_OPTIONAL_RESIZE_FILESYSTEM 0x0010
#define EXT2_OPTIONAL DIR_HASH_INDEX 0x0020

#define EXT2_REQUIRED_COMPRESSION_USED 0x0001
#define EXT2_REQUIRED_DIRENT_CONTAIN_TYPES 0x0002
#define EXT2_REQUIRED_JOURNAL_REPLAY 0x0004
#define EXT2_REQUIRED_FS_USES_JOURNAL_DEVICE 0x0010

#define EXT2_READONLY_SSB_GDT 0x0001
#define EXT2_READONLY_FS_64B_SIZE 0x0002
#define EXT2_READONLY_DIR_CONTENTS_AS_BINARY_TREE 0x0004

#define EXT2_DIRECT_BLOCKS 12
#define EXT2_INDIRECT_BLOCKS 3

#define EXT2_INODE_TYPE_FIFO 0x1000
#define EXT2_INODE_TYPE_CHRDEV 0x2000
#define EXT2_INODE_TYPE_DIR 0x4000
#define EXT2_INODE_TYPE_BLKDEV 0x6000
#define EXT2_INODE_TYPE_REG 0x8000
#define EXT2_INODE_TYPE_SYMLINK 0xA000
#define EXT2_INODE_TYPE_UNIXSOCK 0xC000

#define EXT2_PERMISSION_OTHER_EXECUTE 0x001
#define EXT2_PERMISSION_OTHER_WRITE 0x002
#define EXT2_PERMISSION_OTHER_READ 0x004
#define EXT2_PERMISSION_GROUP_EXECUTE 0x008
#define EXT2_PERMISSION_GROUP_WRITE 0x010
#define EXT2_PERMISSION_GROUP_READ 0x020
#define EXT2_PERMISSION_USER_EXECUTE 0x040
#define EXT2_PERMISSION_USER_WRITE 0x080
#define EXT2_PERMISSION_USER_READ 0x100
#define EXT2_PERMISSION_STICKY_BIT 0x200
#define EXT2_PERMISSION_SET_GID 0x400
#define EXT2_PERMISSION_SET_UID 0x800

#define EXT2_DEFAULT_BLOCK_SIZE 1024
#define EXT2_BGD_BLOCK 2

#define EXT2_ROOT_INODE 2

#define EXT2_SIGNATURE 0xEF53

#define EXT2_DIRENT_TYPE_UNKNOWN 0x00
#define EXT2_DIRENT_TYPE_REGULAR 0x01
#define EXT2_DIRENT_TYPE_DIRECTORY 0x02
#define EXT2_DIRENT_TYPE_CHARDEV 0x03
#define EXT2_DIRENT_TYPE_BLOCKDEV 0x04
#define EXT2_DIRENT_TYPE_FIFO 0x05
#define EXT2_DIRENT_TYPE_SOCKET 0x06
#define EXT2_DIRENT_SYMLINK 0x07

typedef struct ext2_superblock_struct
{
    uint32_t total_inodes;
    uint32_t total_blocks;
    uint32_t reserved_blocks;
    uint32_t free_blocks;
    uint32_t free_inodes;
    uint32_t first_block;
    uint32_t log_block_size;
    uint32_t log_frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t last_mnt_time;
    uint32_t last_write_time;

    uint16_t mount_num;
    uint16_t max_mount;
    uint16_t signature;
    uint16_t fs_state;
    uint16_t err_catch;
    uint16_t version_minor;
    uint32_t last_fsck_time;
    uint32_t interval_between_fsck;
    uint32_t os_id;
    uint32_t version_major;
    uint16_t res_uid;
    uint16_t res_gid;

    // Extended features [ if version_major >= 1 ]
    uint32_t first_inode;
    uint16_t inode_size;
    uint16_t superblock_group;
    uint32_t optional_features;
    uint32_t required_features;
    uint32_t readonly_features;
    uint8_t  fs_id[16];
    uint8_t  vol_id[16];
    uint8_t  mnt_path[64];
    uint32_t compression;
    uint8_t  preallocate_blocks;
    uint8_t  preallocate_dir_blocks;
    uint16_t unused1;
    uint8_t  journal_id[16];
    uint32_t journal_inode;
    uint32_t journal_device;
    uint32_t orphan_head;
    uint8_t  unused2[1024 - 236];
}__attribute__(( packed )) ext2_sb_t;


typedef struct ext2_block_group_descriptor_struct
{
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks;
    uint16_t free_inodes;
    uint16_t num_dirs;
    uint8_t  unused[32-18];
}__attribute__(( packed )) ext2_bgd_t;

typedef struct ext2_inode_struct
{
    uint16_t permission;
    uint16_t uid;
    uint32_t size_lo;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t hard_links;
    uint32_t sectors_used;
    uint32_t flags;
    uint32_t os_specific1;
    uint32_t blocks[EXT2_DIRECT_BLOCKS+EXT2_INDIRECT_BLOCKS];
    uint32_t generation;
    uint32_t file_acl;
    union
    {
        uint32_t dir_acl;
        uint32_t size_high;
    };
    uint32_t fragment_block_addr;
    uint8_t os_specific2[12];
}__attribute__(( packed )) ext2_inode_t;

typedef struct ext2_directory_entry_struct
{
    uint32_t inode;
    uint16_t total_size;
    uint8_t  name_len;
    uint8_t  type;
    uint8_t  name[];
}__attribute__(( packed )) ext2_dirent_t;

typedef struct ext2_fs_struct
{
    FILE* root_node;
    FILE* device;
    ext2_sb_t* sb;
    ext2_bgd_t* bgd;
    uint32_t inodes_per_group;
    uint16_t inode_size;
    uint32_t block_size;
    uint32_t total_blocks;
    uint32_t bgd_blocks;
    uint32_t blocks_per_group;
    uint32_t total_groups;
}ext2_fs_t;

void ext2_read_superblock(ext2_fs_t* fs);

char* ext2_read_inode_block(ext2_fs_t* fs, ext2_inode_t* inode, uint32_t inode_block);
void ext2_write_inode_block(ext2_fs_t* fs, ext2_inode_t* inode, uint32_t inode_block, char* buffer);

void ext2_read_disk_block(ext2_fs_t* fs, uint32_t block, char* buffer);
void ext2_write_disk_block(ext2_fs_t* fs, uint32_t block, char* buffer);

void ext2_read_inode_metadata(ext2_fs_t* fs, ext2_inode_t* inode, uint32_t inode_idx);
void ext2_write_inode_metadata(ext2_fs_t* fs, ext2_inode_t* inode, uint32_t inode_idx);

uint32_t ext2_read_inode_filedata(ext2_fs_t* fs, ext2_inode_t* inode, uint32_t offset, uint32_t size, char* buffer);
void ext2_write_inode_filedata(ext2_fs_t* fs, ext2_inode_t* inode, uint32_t inode_idx, uint32_t offset, uint32_t size, char* buffer);

void ext2_rewrite_bgd(ext2_fs_t* fs);
void ext2_rewrite_superblock(ext2_fs_t* fs);

int ext2_allocate_inode_metadata_block(ext2_fs_t *fs, ext2_inode_t* inode, uint32_t *block_ptr,uint32_t inode_idx, char *buffer, uint32_t block_overwrite);

uint32_t ext2_get_direct_block(ext2_fs_t* fs, ext2_inode_t* inode, uint32_t inode_block);
void ext2_set_direct_block(ext2_fs_t* fs, ext2_inode_t* inode, uint32_t inode_idx, uint32_t inode_block, uint32_t disk_block);

uint32_t ext2_allocate_block(ext2_fs_t* fs);
void ext2_free_block(ext2_fs_t* fs, uint32_t block);

void ext2_allocate_inode_block(ext2_fs_t* fs, ext2_inode_t* inode, uint32_t inode_idx, uint32_t block);
void ext2_free_inode_block(ext2_fs_t* fs, ext2_inode_t* inode, uint32_t inode_idx, uint32_t block);

uint32_t ext2_allocate_inode(ext2_fs_t* fs);
void ext2_free_inode(ext2_fs_t* fs, uint32_t inode_idx);

void ext2_get_root(ext2_fs_t* fs);
FILE* ext2_dirent_to_node(ext2_fs_t *fs, ext2_dirent_t *dir, ext2_inode_t *inode);

uint32_t ext2_getFileSize(FILE* node);

void ext2_mkdir(FILE* parent, char* name, uint16_t permission);
void ext2_mkfile(FILE* parent, char* name, uint16_t permission);

void ext2_createEntry(FILE* parent, char* name, uint32_t entry_inode);
void ext2_removeEntry(FILE* parent, char* name);

void ext2_unlink(FILE* parent, char* name);

char** ext2_listdir(FILE* node);
FILE* ext2_finddir(FILE* parent, char* name);

void ext2_chmod(FILE* node, uint32_t mode);

uint32_t ext2_read(FILE *file, uint32_t offset, uint32_t size, char *buffer);
uint32_t ext2_write(FILE *file, uint32_t offset, uint32_t size, char *buffer);

void ext2_open(FILE *file, uint32_t flags);
void ext2_close(FILE* file);

int isext2(char* dev);

int init_ext2(char* dev, char* mountpoint);

#endif /*__EXT2_H__*/