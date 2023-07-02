#include "ifb.h"
#include "timer.h"
#include "logdisk.h"
#include "stb_image_write.h"
#include "limine_terminal/stb_image.h"
#include "kernelfs.h"

ifb_block_t *block = NULL;

int useIFB = 1;
int IFBinit = 0;

int scrshot_num = 0;

#ifndef __IFB_SCREENSHOT_COMPRESSION__
#define __IFB_SCREENSHOT_COMPRESSION__ 100
#endif

FILE* fbdev = NULL;

uint32_t ifb_read(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    if(offset+size > block->size)
        return -1;
    if(buffer == NULL)
        return -1;
    memcpy(buffer, ((void*)block->buffer)+offset, size);
    return size;
}

uint32_t ifb_write(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    if(offset+size > block->size)
        return -1;
    if(buffer == NULL)
        return -1;
    memcpy(((void*)block->buffer)+offset, buffer, size);
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
        i->width = block->info->width;
        i->height = block->info->height;
        i->pitch = block->info->pitch;
        i->size = block->info->size;
        return 1;
        break;
    default:
        return -1;
        break;
    };
}

void register_video_driver(ifb_video_driver_t* drv)
{
    if(validate(drv) != 1)
        return;

    if(validate(block) != 1)
        return;

    if(validate(block->drivers) != 1)
        return;

    drv->self = list_insert_front(block->drivers, drv);

    if(block->current_driver == NULL)
    {
        block->current_driver = drv;

        block->info = drv->get_modeinfo();
        if(validate(block->info) != 1)
        {
            list_remove_node(block->drivers, drv->self);
            drv->self = NULL;
            return;
        }

        block->buffer = kmalloc_a(block->info->size);
        block->size = block->info->size;
        block->bpp = block->info->bpp;
        fbdev->size = block->size;
    }

    kernelfs_addcharf("/proc", "fb", "Driver:\t%s\nXRes:\t%d\nYRes:\t%d\nBitsPerPixel:\t%d\nPitch:\t%d\nAddress:\t0x%x\n", block->current_driver->name, block->info->width, block->info->height, block->info->bpp, block->info->pitch, block->info->fb);
}

void init_ifb()
{
    block = ZALLOC_TYPES(ifb_block_t);
    block->drivers = list_create();
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
    //ldprintf("IFB", LOG_DEBUG, "intermediate framebuffer initialized.\nheight = %dpx\nwidth = %dpx\nbuffer size = %dB\nframebuffer address: 0x%06x\n", block->info->height, block->info->width, block->size, (uint32_t)block->buffer);
    register_wakeup_callback(ifb_refresh, 60.0/get_frequency());
}

int video_modeset(uint32_t width, uint32_t height, uint32_t bpp)
{
    if(validate(block) != 1)
        return -1;
    if(validate(block->current_driver) != 1)
        return -1;
    if(list_size(block->drivers) == 0)
        return -1;
    if(validate(block->info) != 1)
        return -1;
    if(!(block->current_driver->modeset))
        return -1;

    int ret = block->current_driver->modeset(width, height, bpp);

    if(ret != 0)
    {
        ldprintf("IFB", LOG_ERR, "An error unknown occurred in the current video driver \"%s\" when trying to modeset (error code: %d)", block->current_driver->name, ret);
        return ret;
    }
    
    block->info = block->current_driver->get_modeinfo();
    if(block->current_driver->modeset)
        return block->current_driver->onModeset(block->info);

    return 0;
}

typedef struct
{
    int last_pos;
    void *context;
} custom_stbi_mem_context;

static void custom_stbi_write_mem(void *context, void *data, int size)
{
   custom_stbi_mem_context *c = (custom_stbi_mem_context*)context; 
   char *dst = (char *)c->context;
   char *src = (char *)data;
   int cur_pos = c->last_pos;
   for (int i = 0; i < size; i++)
   {
       dst[cur_pos++] = src[i];
   }
   c->last_pos = cur_pos;
}

void ifb_screenshot()
{
    if(validate(block) != 1)
        return;
    if(validate(block->current_driver) != 1)
        return;
    if(list_size(block->drivers) == 0)
        return;
    if(validate(block->info) != 1)
        return;
    
    ASM_FUNC("cli");
    ldprintf("IFB", LOG_INFO, "Screenshotting..");
    custom_stbi_mem_context ct;
    uint8_t* simg = zalloc(2*MB);
    ct.context = simg;
    ct.last_pos = 0;
    int r = stbi_write_jpg_to_func(custom_stbi_write_mem, &ct, block->info->width, block->info->height, STBI_rgb_alpha, argb_to_brga(block->buffer, block->info->width, block->info->height), __IFB_SCREENSHOT_COMPRESSION__);
    ldprintf("IFB", LOG_INFO, "Size: %dB", ct.last_pos);
    if(r == 1)
    {
        char* buf = zalloc(strlen("screenshot000.jpg"));
        sprintf(buf, "/screenshot%d.jpg", scrshot_num);
        scrshot_num++;
        FILE* f = file_open(buf, OPEN_WRONLY);
        free(buf);
        if(f == NULL)
            return;
        vfs_write(f, 0, ct.last_pos, simg);
        vfs_close(f);
        free(simg);
        ldprintf("IFB", LOG_INFO, "Screenshot saved to %s.", buf);
        free(buf);
    }
    ASM_FUNC("sti");
}

void* ifb_getIFB()
{
    return ((void*)block->buffer);
}

void ifb_refresh()
{
    // if(validate(block) != 1)
    //     return;
    // if(validate(block->current_driver) != 1)
    //     return;

    if(block->info == NULL)
        return;

    int ret = block->current_driver->draw(block->buffer, block->info->width, block->info->height);

    if(ret != 0)
    {
        ldprintf("IFB", LOG_ERR, "An unknown error has been occurred in the current video driver when drawing frame");
        return;
    }
}

uint32_t video_get_width()
{
    if(validate(block) != 1)
        return 0;
    if(validate(block->current_driver) != 1)
        return 0;
    if(list_size(block->drivers) == 0)
        return 0;
    if(validate(block->info) != 1)
        return 0;
    if(!(block->current_driver->get_modeinfo))
        return 0;

    block->info = block->current_driver->get_modeinfo();
    if(validate(block->info) != 1)
        return 0;

    return block->info->width;
}

uint32_t video_get_height()
{
    if(validate(block) != 1)
        return 0;
    if(validate(block->current_driver) != 1)
        return 0;
    if(list_size(block->drivers) == 0)
        return 0;
    if(validate(block->info) != 1)
        return 0;
    if(!(block->current_driver->get_modeinfo))
        return 0;

    block->info = block->current_driver->get_modeinfo();
    if(validate(block->info) != 1)
        return 0;

    return block->info->height;
}

uint32_t video_get_bpp()
{
    if(validate(block) != 1)
        return 0;
    if(validate(block->current_driver) != 1)
        return 0;
    if(list_size(block->drivers) == 0)
        return 0;
    if(validate(block->info) != 1)
        return 0;
    if(!(block->current_driver->get_modeinfo))
        return 0;

    block->info = block->current_driver->get_modeinfo();
    if(validate(block->info) != 1)
        return 0;

    return block->info->bpp;
}

ifb_video_info_t* video_get_modeinfo()
{
    if(validate(block) != 1)
        return NULL;
    if(validate(block->current_driver) != 1)
        return NULL;
    if(list_size(block->drivers) == 0)
        return NULL;
    if(!(block->current_driver->get_modeinfo))
        return NULL;

    return block->current_driver->get_modeinfo();
}

void register_modeset_handler(ifb_video_onModeset onModeset)
{
    if(validate(block) != 1)
        return;
    if(validate(block->current_driver) != 1)
        return;
    if(list_size(block->drivers) == 0)
        return;

    block->current_driver->onModeset = onModeset;
    return;
}

int video_draw(uint32_t* fb)
{
    if(validate(block) != 1)
        return -1;
    if(validate(block->current_driver) != 1)
        return -1;
    if(list_size(block->drivers) == 0)
        return -1;
    if(!(block->current_driver->draw))
        return -1;

    return block->current_driver->draw(fb, block->info->width, block->info->height);
}