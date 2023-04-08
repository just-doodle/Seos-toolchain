#include "ramdisk.h"

int c_ramdisk = 0;

uint32_t ramdisk_read(FILE* f, uint32_t offset, size_t size, char* buffer)
{
    ramdisk_t* rdisk = f->device;
    if(offset + size > rdisk->size)
        return -1;
    memcpy(buffer, (void*)(rdisk->start+offset), size);
    return size;
}

uint32_t ramdisk_write(FILE* f, uint32_t offset, size_t size, char* buffer)
{
    ramdisk_t* rdisk = f->device;
    if(offset + size > rdisk->size || rdisk->is_writable != 1)
        return -1;
    memcpy((void*)(rdisk->start+offset), buffer, size);
    return size;
}

void ramdisk_open(FILE* f, uint32_t flags)
{
    return;
}

void ramdisk_close(FILE* f)
{
    return;
}

uint32_t ramdisk_getfilesize(FILE* f)
{
    ramdisk_t* r = f->device;
    return r->size;
}

FILE* get_node(ramdisk_t* ramdisk)
{
    FILE* f = ZALLOC_TYPES(FILE);

    strcpy(f->name, "rdisk");
    strcat(f->name, itoa_r(ramdisk->num, 10));
    f->inode_num = ramdisk->num;
    f->device = ramdisk;
    f->size = ramdisk->size;
    f->open = ramdisk_open;
    f->close = ramdisk_close;
    f->write = ramdisk_write;
    f->read = ramdisk_read;
    f->get_filesize = ramdisk_getfilesize;

    f->flags = FS_BLOCKDEVICE;

    return f;
}

void add_ramdisk(uint32_t start, uint32_t end, int isWritable)
{
    ramdisk_t* rdisk = ZALLOC_TYPES(ramdisk_t);
    rdisk->start = start;
    rdisk->is_writable = isWritable;
    rdisk->end = end;
    rdisk->size = (rdisk->end - rdisk->start);
    rdisk->num = c_ramdisk;
    c_ramdisk++;

    devfs_add(get_node(rdisk));
    return;
}

int isRamdiskCreated(int num)
{
    if(num < c_ramdisk)
    {
        return 1;
    }
    return 0;
}

void add_ramdisk_file(char* file, int isWritable)
{
    FILE* f = file_open(file, 0);
    if(f == NULL)
    {
        printf("[RAMDISK] File not found: %s\n", file);
        return;
    }
    if(strstr(file, "rdisk") != NULL)
    {
        return;
    }

    size_t sz = vfs_getFileSize(f);

    char* b = zalloc(sz);
    vfs_read(f, 0, sz, b);
    add_ramdisk(((uint32_t)b), ((uint32_t)b) + sz, 1);
    vfs_close(f);
    return;
}