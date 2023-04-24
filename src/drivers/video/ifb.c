#include "ifb.h"

ifb_block_t *block = NULL;

void init_ifb()
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

    serialprintf("[ifb] intermediate framebuffer initialized.\nheight = %dpx\nwidth = %dpx\nbuffer size = %dB\nframebuffer address: 0x%06x\n", block->fb_height, block->fb_width, block->size, (uint32_t)block->buffer);

    pit_register(ifb_refresh, 60 / pit_frequency);
}

void* ifb_getIFB()
{
    return block->buffer;
}

void ifb_refresh()
{
    fast_memcpy(block->fb_addr, block->buffer, block->size);
}