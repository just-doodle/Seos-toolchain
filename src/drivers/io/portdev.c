#include "portdev.h"

#define PORT_DEVICE_FILENAME "port"

void portdev_open(FILE* f, uint32_t flags)
{
    return;
}

void portdev_close(FILE* f)
{
    return;
}

uint32_t portdev_read(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    switch(size)
    {
    case 1:
        buffer[0] = inb(offset & 0xFFFF);
        break;
    case 2:
        ((uint16_t*)(buffer))[0] = inw(offset & 0xFFFF);
        break;
    case 4:
        ((uint32_t*)(buffer))[0] = inl(offset & 0xFFFF);
        break;
    default:
        for(int i = 0; i < size; i++)
            buffer[i] = inb(offset & 0xFFFF);
        break;
    };
    serialprintf("[PORT] READ 0x%x\n", buffer[0]);
    return size;
}

uint32_t portdev_write(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    switch(size)
    {
    case 1:
        outb(offset & 0xFFFF, buffer[0]);
        break;
    case 2:
        outw(offset & 0xFFFF, ((uint16_t*)buffer)[0]);
        break;
    case 4:
        outl(offset & 0xFFFF, ((uint32_t*)buffer)[0]);
        break;
    default:
        for(uint32_t i = 0; i < size; i++)
            outb(offset & 0xFFFF, buffer[i]);
        break;
    };
    return size;
}

void init_portDev()
{
    FILE* portdev = ZALLOC_TYPES(FILE);
    strcpy(portdev->name, PORT_DEVICE_FILENAME);
    portdev->open = portdev_open;
    portdev->close = portdev_close;
    portdev->read = portdev_read;
    portdev->write = portdev_write;
    portdev->flags = FS_BLOCKDEVICE;
    portdev->size = 0xFFFF;

    devfs_add(portdev);
}