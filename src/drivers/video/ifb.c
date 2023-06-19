#include "ifb.h"
#include "timer.h"
#include "logdisk.h"
#include "stb_image_write.h"
#include "limine_terminal/stb_image.h"

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
        block->fb_ptr = vesa_getFramebuffer();

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

        register_wakeup_callback(ifb_refresh, 60.0/get_frequency());
        IFBinit = 1;
    }
    else
    {
    }
}

void ifb_change_res(uint32_t width, uint32_t height, uint32_t bpp)
{
    if(useIFB == 1)
    {
        asm("cli");
        VBE_MODE_INFO_t* minfo = vesa_get_current_mode();
        free_region(kernel_page_dir, minfo->phys_base, minfo->phys_base + ((minfo->pitch * minfo->YResolution)), 1);
        block->bpp = bpp;
        block->fb_bpp = bpp;
        block->fb_width = width;
        block->fb_height = height;
        block->fb_pitch = (width*bpp);
        block->fb_size = block->fb_pitch * bpp/8;
        block->buffer = realloc(block->buffer, block->fb_size);
        block->size = block->fb_size;
        alloc_region(kernel_page_dir, minfo->phys_base, minfo->phys_base + (block->fb_pitch * block->fb_height), 1, 1, 1);
        compositor_change_res(width, height, bpp);
        asm("sti");
    }
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
    if(useIFB == 1)
    {
        asm("cli");
        ldprintf("IFB", LOG_INFO, "Screenshotting..");
        custom_stbi_mem_context ct;
        uint8_t* simg = zalloc(2*MB);
        ct.context = simg;
        ct.last_pos = 0;

        int r = stbi_write_jpg_to_func(custom_stbi_write_mem, &ct, block->fb_width, block->fb_height, STBI_rgb_alpha, argb_to_brga(block->buffer, block->fb_width, block->fb_height), __IFB_SCREENSHOT_COMPRESSION__);
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
        asm("sti");
    }
}

void* ifb_getIFB()
{
    return ((void*)block->buffer);
}

void ifb_refresh()
{
    if(useIFB == 1)
    {
        // for(register int y = 0; y < block->fb_height; y++)
        //     for(register int x = 0; x < block->fb_width; x++)
        //         block->fb_ptr[x + y * block->fb_width] = block->buffer[x + y * block->fb_width];
        vesa_copy_framebuffer(block->buffer);
    }
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