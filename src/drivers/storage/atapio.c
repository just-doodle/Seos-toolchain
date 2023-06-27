#include "atapio.h"
#include "mbr.h"
#include "logdisk.h"

ata_pio_t ap0m;
ata_pio_t ap0s;
ata_pio_t ap1m;
ata_pio_t ap1s;

void ata_pio_handler(registers_t* reg)
{
}

void ata_piodev_init(ata_pio_t* ap, bool primary, bool master)
{
    ap->master = master;
    ap->basePort = primary ? 0x1F0 : 0x170;
    ap->dataPort = ap->basePort;
    ap->errorPort = ap->basePort + 1;
    ap->sectorCountPort = ap->basePort + 2;
    ap->lbaLowPort = ap->basePort + 3;
    ap->lbaMidPort = ap->basePort + 4;
    ap->lbaHiPort = ap->basePort + 5;
    ap->devicePort = ap->basePort + 6;
    ap->commandPort = ap->basePort + 7;
    ap->controlPort = ap->basePort + 0x206;
    ap->bytesPerSector = 512;

    ata_pio_identify(ap);
}

void init_ata_pio()
{
    register_interrupt_handler(0x2E, ata_pio_handler);
    register_interrupt_handler(0x2F, ata_pio_handler);

    // Init Primary
    ata_piodev_init(&ap0m, true, true);
    ata_piodev_init(&ap0s, true, false);

    // Init Secondary
    ata_piodev_init(&ap1m, false, true);
    ata_piodev_init(&ap1s, false, false);
}

void ata_pio_identify(ata_pio_t* ap)
{
    outb(ap->devicePort, ap->master ? 0xA0 : 0xB0);
    outb(ap->controlPort, 0);

    outb(ap->devicePort, 0xA0);
    uint8_t status = inb(ap->commandPort);
    if(status == 0xFF)
    {
        return;
    }

    outb(ap->devicePort, ap->master ? 0xA0 : 0xB0);
    outb(ap->sectorCountPort, 0);
    outb(ap->lbaLowPort, 0);
    outb(ap->lbaMidPort, 0);
    outb(ap->lbaHiPort, 0);
    outb(ap->commandPort, 0xEC);

    status = inb(ap->commandPort);

    if(status == 0x00)
    {
        printf("[ATA PIO] Device not found\n");
        return;
    }

    while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
        status = inb(ap->commandPort);

    if(status & 0x01)
    {
        printf("[ATA PIO] Error when Polling\n");
        return;
    }

    uint16_t * id = (uint16_t*)kmalloc(256);
    memset(id, 0, 256);
    uint8_t * id_buf = (uint8_t*)id;
    //serialprintf("-------START ATA %d IDENTIFY--------\n", (ap->basePort == 0x1F0) ? (ap->master ? 0 : 1) : (ap->master ? 2 : 3));
    for(int i = 0; i < 256; i++)
    {
        id[i] = inw(ap->dataPort);
        //serialprintf("%02x ", id_buf[i]);
    }
    //serialprintf("\n-------END ATA %d IDENTIFY--------\n", (ap->basePort == 0x1F0) ? (ap->master ? 0 : 1) : (ap->master ? 2 : 3));
    printf("[ATA PIO] Device found\n");
    
    ap->channel = 1;
    ap->type = 0;
    ap->signature = *(uint16_t*)(id_buf + IDENT_DEVICETYPE);
    ap->capabilities = *(uint16_t*)(id_buf + IDENT_CAPABILITIES);
    ap->commandSets = *(uint16_t*)(id_buf + IDENT_COMMANDSETS);

    if(ap->commandSets & (1 << 26))
    {
        printf("[ATA PIO] LBA 48 supported\n");
        ap->size = *(uint32_t*)(id_buf + IDENT_MAX_LBA_EXT);
    }
    else
    {
        printf("[ATA PIO] LBA28 or CHS supported\n");
        ap->size = *(uint32_t*)(id_buf + IDENT_MAX_LBA);
    }

    memset(ap->model, 0, 41);

    for(int i = 0; i < 40; i += 2)
    {
        ap->model[i] = id_buf[54 + i + 1];
        ap->model[i + 1] = id_buf[54 + i];
    }

    ap->model[40] = 0;

    ldprintf("ATA PIO", LOG_INFO, "Device found on %s %s", ap->basePort == 0x1F0 ? "Primary" : "Secondary", ap->master ? "Master" : "Slave");
    ldprintf("ATA PIO", LOG_INFO, "Model: %s", ap->model);
    ldprintf("ATA PIO", LOG_INFO, "Size: %s", stoc_r(ap->size*512));
    ldprintf("ATA PIO", LOG_INFO, "Signature: 0x%x", ap->signature);

    FILE* f = ata_pio_GetVFSNode(ap);

    devfs_add(f);
    char* name = kmalloc(strlen(f->name) + strlen("/dev/"));
    memset(name , 0, strlen(f->name) + strlen("/dev/"));
    strcpy(name, "/dev/");
    strcat(name, f->name);
    serialprintf("%s\n", name);

    probe_partitions(name);

    kfree(name);
}

void ata_pio_Read28(ata_pio_t* ap, uint32_t lba, uint8_t* buffer, int count)
{
    outb(ap->devicePort, ap->master ? 0xE0 : 0xF0 |  ((lba & 0x0F000000) >> 24));
    outb(ap->errorPort, 0);

    outb(ap->sectorCountPort, 1);
    outb(ap->lbaLowPort, lba & 0x000000FF);
    outb(ap->lbaMidPort, (lba & 0x0000FF00) >> 8);
    outb(ap->lbaHiPort, (lba & 0x00FF0000) >> 16);

    outb(ap->commandPort, 0x20);

    uint8_t status = inb(ap->commandPort);
    while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
        status = inb(ap->commandPort);

    if(status & 0x01)
    {
        printf("[ATA PIO] Error when reading\n");
        return;
    }

    for(int i = 0; i < count; i += 2)
    {
        uint16_t wdata = inw(ap->dataPort);
        buffer[i] = wdata & 0x00FF;
        if (i + 1 < count)
            buffer[i + 1] = (wdata >> 8) & 0x00FF;
    }

    for (uint16_t i = count + (count % 2); i < ap->bytesPerSector; i += 2) inw(ap->dataPort);
}

void ata_pio_Write28(ata_pio_t* ap, uint32_t lba, uint8_t* buffer, int count)
{
    outb(ap->devicePort, ap->master ? 0xE0 : 0xF0 |  ((lba & 0x0F000000) >> 24));
    outb(ap->errorPort, 0);

    outb(ap->sectorCountPort, 1);
    outb(ap->lbaLowPort, lba & 0x000000FF);
    outb(ap->lbaMidPort, (lba & 0x0000FF00) >> 8);
    outb(ap->lbaHiPort, (lba & 0x00FF0000) >> 16);

    outb(ap->commandPort, 0x30);

    uint8_t status = inb(ap->commandPort);
    while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
        status = inb(ap->commandPort);

    if(status & 0x01)
    {
        printf("[ATA PIO] Error when Writing\n");
        return;
    }

    for(int i = 0; i < count; i += 2)
    {
        uint16_t wdata = buffer[i];
        if (i + 1 < count)
            wdata |= buffer[i + 1] << 8;
        outw(ap->dataPort, wdata);
    }

    for (uint16_t i = count + (count % 2); i < ap->bytesPerSector; i += 2) outw(ap->dataPort, 0);
}

void ata_pio_Flush28(ata_pio_t* ap)
{
    outb(ap->devicePort, ap->master ? 0xE0 : 0xF0);
    outb(ap->commandPort, 0xE7);
    uint8_t status = inb(ap->commandPort);
    while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
        status = inb(ap->commandPort);

    if(status & 0x01)
    {
        printf("[ATA PIO] Error when Flushing\n");
        return;
    }
}

char* ata_pio_read_sector(ata_pio_t* ap, uint32_t lba)
{
    char* buffer = (char*)zalloc(ap->bytesPerSector);
    ata_pio_Read28(ap, lba, (uint8_t*)buffer, ap->bytesPerSector);
    return buffer;
}

void ata_pio_write_sector(ata_pio_t* ap, uint32_t lba, uint8_t* buffer)
{
    ata_pio_Write28(ap, lba, buffer, ap->bytesPerSector);
    ata_pio_Flush28(ap);
}

uint32_t ata_pio_write(vfs_node* node, uint32_t offset, uint32_t size, char* buffer)
{
    ata_pio_t* ap = (ata_pio_t*)node->device;
    uint32_t start = offset / ap->bytesPerSector;
    uint32_t start_offset = offset % ap->bytesPerSector;

    uint32_t end = (offset + size - 1) / ap->bytesPerSector;
    uint32_t end_offset = (offset + size - 1) % ap->bytesPerSector;

    char * buf_curr = buffer;
    uint32_t counter = start;
    uint32_t write_size;
    uint32_t off, total = 0;

    while(counter <= end)
    {
        off = 0;
        write_size = ap->bytesPerSector;
        char * ret = ata_pio_read_sector(ap, counter);
        if(counter == start)
        {
            off = start_offset;
            write_size = ap->bytesPerSector - off;
        }
        if(counter == end)
        {
            write_size = end_offset - off + 1;
        }
        memcpy(ret + off, buf_curr, write_size);
        ata_pio_write_sector(ap, counter, (uint8_t*)ret);
        buf_curr = buf_curr + write_size;
        total = total + write_size;
        counter++;
    }
    return total;
}

uint32_t ata_pio_read(vfs_node* node, uint32_t offset, uint32_t size, char* buffer)
{
    asm("cli");
    ata_pio_t* ap = (ata_pio_t*)node->device;
    uint32_t start = offset / ap->bytesPerSector;
    uint32_t start_offset = offset % ap->bytesPerSector;

    uint32_t end = (offset + size - 1) / ap->bytesPerSector;
    uint32_t end_offset = (offset + size - 1) % ap->bytesPerSector;

    char * buf_curr = buffer;
    uint32_t counter = start;
    uint32_t read_size;
    uint32_t off, total = 0;

    while(counter <= end)
    {
        off = 0;
        read_size = ap->bytesPerSector;

        char * ret = ata_pio_read_sector(ap, counter);

        if(counter == start)
        {
            off = start_offset;
            read_size = ap->bytesPerSector - off;
        }
        if(counter == end)
            read_size = end_offset - off + 1;

        memcpy(buf_curr, ret + off, read_size);
        free(ret);
        buf_curr = buf_curr + read_size;
        total = total + read_size;
        counter++;
    }
    asm("sti");
    return total;
}

void ata_pio_open(vfs_node* node, uint32_t flags)
{
    return;
}

void ata_pio_close(vfs_node* node)
{
    return;
}

FILE* ata_pio_GetVFSNode(ata_pio_t* ap)
{
    FILE* f = (FILE*)kcalloc(sizeof(FILE), 1);
    f->device = ap;
    f->read = ata_pio_read;
    f->write = ata_pio_write;
    f->open = ata_pio_open;
    f->close = ata_pio_close;
    f->size = ap->size;
    strcpy(f->name, "apio");
    strcat(f->name, ((ap->basePort == 0x1F0) ? (ap->master ? "0" : "1") : (ap->master ? "2" : "3")));
    f->flags = FS_BLOCKDEVICE;
    return f;
}