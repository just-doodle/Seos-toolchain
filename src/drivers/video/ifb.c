#include "ifb.h"
#include "timer.h"

ifb_block_t *block = NULL;

int useIFB = 1;
int IFBinit = 0;

FILE* fbdev = NULL;

uint32_t ifb_read(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    if(offset+size > block->size)
        return -1;
    if(buffer == NULL)
        return -1;
    memcpy(buffer, ((void*)block->fb_addr)+offset, size);
    return size;
}

uint32_t ifb_write(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    if(offset+size > block->size)
        return -1;
    if(buffer == NULL)
        return -1;
    memcpy(((void*)block->fb_addr)+offset, buffer, size);
    return size;
}

uint32_t ifb_getfilesize(FILE* f)
{
    return block->size;
}

void ifb_open(FILE* f, uint32_t flags)
{
    return;
}

void ifb_close(FILE* f)
{
    return;
}

typedef struct fb_info_struct
{
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t pitch;
    uint32_t size;
    uint32_t reserved[6];
}fb_info_t;

int ifb_ioctl(FILE* f, int request, void *data)
{
    switch(request)
    {
    case 153:
        fb_info_t* i = data;
        i->bpp = block->bpp;
        i->width = block->fb_width;
        i->height = block->fb_height;
        i->pitch = block->fb_pitch;
        i->size = block->size;
        return 1;
        break;
    default:
        return -1;
        break;
    };
}

void init_ifb()
{
    if(useIFB == 1)
    {
        block = ZALLOC_TYPES(ifb_block_t);

        VBE_MODE_INFO_t *mode = vesa_get_current_mode();

        block->fb_width = mode->XResolution;
        block->fb_height = mode->YResolution;
        block->fb_bpp = mode->bpp;
        block->fb_addr = mode->phys_base;
        block->fb_pitch = mode->pitch;

        block->size = block->fb_pitch * block->fb_height;
        block->fb_size = block->fb_pitch * block->fb_height;
        block->bpp = 32;

        block->buffer = zalloc(block->size);

        fbdev = ZALLOC_TYPES(FILE);
        strcpy(fbdev->name, "fb");
        fbdev->get_filesize = ifb_getfilesize;
        fbdev->write = ifb_write;
        fbdev->read = ifb_read;
        fbdev->open = ifb_open;
        fbdev->close = ifb_close;
        fbdev->size = block->size;
        fbdev->ioctl = ifb_ioctl;
        fbdev->flags = FS_BLOCKDEVICE;

        devfs_add(fbdev);

        serialprintf("[ifb] intermediate framebuffer initialized.\nheight = %dpx\nwidth = %dpx\nbuffer size = %dB\nframebuffer address: 0x%06x\n", block->fb_height, block->fb_width, block->size, (uint32_t)block->buffer);

        register_wakeup_callback(ifb_refresh, 60/get_frequency());
        IFBinit = 1;
    }
    else
    {
    }
}

void* ifb_getIFB()
{
    return ((void*)block->buffer);
}

void ifb_refresh()
{
    if(useIFB == 1)
        fast_memcpy((char*)block->fb_addr, block->buffer, block->size);
}

void toggle_ifb()
{
    int lj = useIFB;
    useIFB = !useIFB;
    if(lj == 0)
    {
        if(IFBinit == 0)
            init_ifb();
    }
    else
    {
    }
}