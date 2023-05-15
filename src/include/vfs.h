#ifndef __vfs_H__
#define __vfs_H__

#include "system.h"
#include "string.h"
#include "list.h"
#include "tree.h"
#include "printf.h"
#include "kheap.h"
#include "pit.h"

#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STRING "/"
#define PATH_UP ".."
#define PATH_CURRENT "."
#define vfs_EXT2_MAGIC 0xEF53

#define OPEN_RDONLY 0x0000
#define OPEN_WRONLY 0x0100
#define OPEN_RDWR 0x0200
#define OPEN_APPEND 0x0800
#define OPEN_CREAT 0x1000
#define OPEN_TRUNC 0x2000
#define OPEN_EXCL 0x4000
#define OPEN_NOFOLLOW 0x1000
#define OPEN_DIRECTORY 0x4000

#define FS_FILE 0x01
#define FS_DIRECTORY 0x02
#define FS_CHARDEVICE 0x04
#define FS_BLOCKDEVICE 0x08
#define FS_PIPE 0x10
#define FS_SYMLINK 0x20
#define FS_MOUNTPOINT 0x40

#define FS_TYPE_EXT2    0x83
#define FS_TYPE_SORFS   0x84

#define _IFMT 0170000   // type of file
#define _IFDIR 0040000  // directory
#define _IFCHR 0020000  // character special
#define _IFBLK 0060000  // block special
#define _IFREG 0100000  // regular
#define _IFLNK 0120000  // symbolic link
#define _IFSOCK 0140000 // socket
#define _IFIFO 0010000  // fifo

struct vfs_node;

typedef uint32_t (*get_filesize_callback)(struct vfs_node *);
typedef uint32_t (*read_callback)(struct vfs_node *, uint32_t, uint32_t, char *);
typedef uint32_t (*write_callback)(struct vfs_node *, uint32_t, uint32_t, char *);
typedef void (*open_callback)(struct vfs_node *, uint32_t flags);
typedef void (*close_callback)(struct vfs_node *);
typedef struct DirectoryEntry *(*readdir_callback)(struct vfs_node *, uint32_t);
typedef struct vfs_node *(*finddir_callback)(struct vfs_node *, char *);
typedef void (*create_callback)(struct vfs_node *, char *name, uint16_t permissions);
typedef void (*unlink_callback)(struct vfs_node *, char *name);
typedef void (*mkdir_callback)(struct vfs_node *, char *name, uint16_t permissions);
typedef int (*ioctl_callback)(struct vfs_node *, int request, void *data);
typedef int (*get_size_callback)(struct vfs_node *);
typedef void (*chmod_callback)(struct vfs_node *, uint32_t mode);
typedef char **(*listdir_callback)(struct vfs_node *);
typedef struct vfs_node *(*vfs_mount_callback)(char *arg, char *mountpoint);

typedef struct vfs_node
{
    char name[256];
    void *device;

    uint32_t mask;
    uint32_t uid;
    uint32_t gid;
    uint32_t flags;
    uint32_t inode_num;
    uint32_t size;
    uint32_t fs_type;
    uint32_t dev_id;
    uint32_t open_flags;

    uint32_t last_modified;
    uint32_t last_accessed;
    uint32_t creation_time;

    uint32_t offset;
    unsigned nlink;
    int ref_count;

    read_callback read;
    write_callback write;
    open_callback open;
    close_callback close;
    readdir_callback readdir;
    finddir_callback finddir;
    create_callback create;
    unlink_callback unlink;
    mkdir_callback mkdir;
    ioctl_callback ioctl;
    get_size_callback get_size;
    chmod_callback chmod;
    listdir_callback listdir;
    get_filesize_callback get_filesize;
}vfs_node;

typedef struct DirectoryEntry
{
    uint32_t inode_count;
    char name[256];
}DirectoryEntry;

typedef struct vfs_entry
{
    char *name;
    vfs_node *file;
}vfs_entry;

typedef vfs_node FILE;

uint32_t vfs_getFileSize(vfs_node *file);
vfs_node *get_mountpoint(char **path);
uint32_t vfs_read(vfs_node *file, uint32_t offset, uint32_t size, char *buffer);
uint32_t vfs_write(vfs_node *file, uint32_t offset, uint32_t size, char *buffer);
void vfs_open(vfs_node *file, uint32_t flags);
void vfs_close(vfs_node *file);
struct vfs_node *vfs_finddir(vfs_node *file, char *name);
void vfs_mkdir(char *name, uint16_t permissions);
void vfs_mkfile(char *name, uint16_t permissions);
int vfs_create(char *name, uint16_t permissions);

DirectoryEntry* vfs_readdir(FILE* f, uint32_t index);

vfs_node *file_open(char *name, uint32_t flags);
char *expand_path(char *path);

void vfs_chmod(vfs_node *file, uint32_t mode);
int vfs_ioctl(vfs_node *file, int request, void *data);

void vfs_unlink(char *name);
void vfs_symlink(char *name, char *target);
int vfs_readlink(vfs_node *node, char *buffer, size_t size);

uint32_t find_fs(char* device);

void init_vfs();
void vfs_mount(char *path, vfs_node *local_root);
void vfs_register_fs(char *name, vfs_mount_callback callm);
void vfs_mountDev(char *mountpoint, vfs_node *node);
void print_vfs_tree();
char** vfs_listdir(char *name);
void vfs_db_listdir(char *name);

void vfs_unmount(char *path);

#endif /*__vfs_H__*/