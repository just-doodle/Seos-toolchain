#include "nulldev.h"

void nulldev_open(FILE* f, uint32_t flags)
{
    return;
}

void nulldev_close(FILE* f)
{
    return;
}

uint32_t nulldev_read(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    return 0;
}

uint32_t nulldev_write(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    return size;
}

void zerodev_open(FILE* f, uint32_t flags)
{
    return;
}

void zerodev_close(FILE* f)
{
    return;
}

uint32_t zerodev_read(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    memset(buffer, 0, size);
    return size;
}

uint32_t zerodev_write(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    return size;
}

void random_open(FILE* f, uint32_t flags)
{
    return;
}

void random_close(FILE* f)
{
    return;
}

uint32_t random_read(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    for(uint32_t i = 0; i < size; i++)
        buffer[i] = rand() % 0xFF;
    return size;
}

void init_nulldev()
{
    FILE* ndev = ZALLOC_TYPES(FILE);
    strcpy(ndev->name, "null");
    ndev->open = nulldev_open;
    ndev->close = nulldev_close;
    ndev->read = nulldev_read;
    ndev->write = nulldev_write;
    ndev->mask = 0666;
    ndev->size = 0;
    ndev->flags = FS_CHARDEVICE;

    FILE* zdev = ZALLOC_TYPES(FILE);
    strcpy(zdev->name, "zero");
    zdev->open = zerodev_open;
    zdev->close = zerodev_close;
    zdev->read = zerodev_read;
    zdev->write = zerodev_write;
    zdev->mask = 0666;
    zdev->size = 0;
    zdev->flags = FS_CHARDEVICE;

    FILE* rdev = ZALLOC_TYPES(FILE);
    strcpy(rdev->name, "random");
    rdev->open = random_open;
    rdev->close = random_close;
    rdev->read = random_read;
    rdev->mask = 0666;
    rdev->size = 0;
    rdev->flags = FS_CHARDEVICE;

    FILE* urdev = ZALLOC_TYPES(FILE);
    strcpy(urdev->name, "urandom");
    urdev->open = random_open;
    urdev->close = random_close;
    urdev->read = random_read;
    urdev->mask = 0666;
    urdev->size = 0;
    urdev->flags = FS_CHARDEVICE;

    devfs_add(ndev);
    devfs_add(zdev);
    devfs_add(rdev);
    devfs_add(urdev);
}