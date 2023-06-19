#ifndef __IFB_H__
#define __IFB_H__

#include "system.h"
#include "kheap.h"
#include "printf.h"
#include "string.h"
#include "pit.h"
#include "ramdisk.h"
#include "vesa.h"

typedef struct ifb_block
{
    uint32_t fb_height;
    uint32_t fb_width;
    uint32_t fb_bpp;
    uint32_t fb_pitch;
    uint32_t fb_addr;
    uint32_t fb_size;

    uint32_t* fb_ptr;

    uint32_t *buffer;
    uint32_t size;
    uint32_t bpp;
}ifb_block_t;

void init_ifb();

bool isIFB_enabled();
void change_IFB_state(bool state);

void ifb_refresh();

void *ifb_getIFB();

#endif /*__IFB_H__*/