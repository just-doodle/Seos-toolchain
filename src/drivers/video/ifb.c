#include "ifb.h"
#include "timer.h"

ifb_block_t *block = NULL;

int useIFB = 1;
int IFBinit = 0;

uint32_t** fb = 0;

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

        *fb = block->buffer;

        serialprintf("[ifb] intermediate framebuffer initialized.\nheight = %dpx\nwidth = %dpx\nbuffer size = %dB\nframebuffer address: 0x%06x\n", block->fb_height, block->fb_width, block->size, (uint32_t)block->buffer);

        register_wakeup_callback(ifb_refresh, 60/get_frequency());
        IFBinit = 1;
    }
    else
    {
        *fb = vesa_getFramebuffer();
    }
}

void* ifb_getIFB()
{
    return ((void*)*fb);
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
        *fb = block->buffer;
    }
    else
    {
        *fb = vesa_getFramebuffer();
    }


}