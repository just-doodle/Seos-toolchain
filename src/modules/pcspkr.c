#include "system.h"
#include "ports.h"
#include "vfs.h"
#include "kheap.h"
#include "pit.h"
#include "devfs.h"

#include "modules.h"

typedef struct note_struct
{
    uint32_t frequency;
    uint32_t length;
}note_t;

static void pcspkr_play(uint32_t frequency)
{
    uint32_t div = 0;
    uint8_t tmp;

    div = PIT_CHANNEL0_FREQUENCY/frequency;
    outb(0x43, 0xb6);
 	outb(0x42, (uint8_t)((div) & 0xFF));
 	outb(0x42, (uint8_t)((div >> 8) & 0xFF));

    tmp = inb(0x61);
    if(tmp != (tmp | 3))
    {
        outb(0x61, tmp | 3);
    }
}

static void pcspkr_nosound()
{
    uint8_t tmp = inb(0x61) & 0xFC;
 	outb(0x61, tmp);
}

static int pcspkr_play_note(note_t* note)
{
    if(validate(note) != 1)
        return 1;
    pcspkr_play(note->frequency);
    sleep(note->length);
    pcspkr_nosound();
    return 0;
}

static void beep(uint32_t frequency, uint32_t len)
{
    pcspkr_play(frequency);
    sleep(len);
    pcspkr_nosound();
}

uint32_t pcspkr_write(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    if((size % sizeof(note_t)) != 0)
        return 0;

    uint32_t error_notes = 0;

    note_t* notes = (note_t*)buffer;
    for(uint32_t i = 0; i < (size / sizeof(note_t)); i++)
    {
        beep(notes[i].frequency, notes[i].length);
    }

    return size - (sizeof(note_t)*error_notes);
}

void pcspkr_open(FILE* f, uint32_t flags)
{
    return;
}

void pcspkr_close(FILE* f)
{
    return;
}

static int init(int argc, char** argv)
{
    FILE* f = ZALLOC_TYPES(FILE);
    if(validate(f) != 1)
        return -1;

    strcpy(f->name, "pcspkr");
    f->write = pcspkr_write;
    f->open = pcspkr_open;
    f->close = pcspkr_close;
    f->flags = FS_CHARDEVICE;
    f->mask = 0666;
    f->size = 1;
    devfs_add(f);

    test_function();

    return 0;
}

static int fini()
{
    pcspkr_nosound();
    vfs_unmount("/dev/pcspkr");
    devfs_remove("/dev/pcspkr");
    return 0;
}

MODULE_DEF(pcspkr, init, fini);
MODULE_DEPENDS(pcspkr, test);