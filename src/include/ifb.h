#ifndef __IFB_H__
#define __IFB_H__

#include "system.h"
#include "kheap.h"
#include "printf.h"
#include "string.h"
#include "pit.h"
#include "ramdisk.h"
#include "vesa.h"

typedef struct ifb_video_info_struct
{
    uint32_t width;
    uint32_t height;
    uint32_t bpp;

    uint32_t pitch;
    uint32_t size;

    uint32_t* fb;

    uint32_t pix;
}ifb_video_info_t;

typedef int (*ifb_video_driver_modeset)(uint32_t width, uint32_t height, uint32_t bpp); // Return 0 if success else 1
typedef ifb_video_info_t* (*ifb_video_driver_get_modeinfo)();
typedef int (*ifb_video_driver_draw)(uint32_t* buffer, uint32_t width, uint32_t height);

typedef int (*ifb_video_onModeset)(ifb_video_info_t* info);

#define IFB_PIXELFORMAT_ARGB 0
#define IFB_PIXELFORMAT_BGRA 1

typedef struct ifb_video_driver_struct
{
    char name[32];
    uint32_t flags; // Reserved for now

    ifb_video_driver_modeset modeset;
    ifb_video_driver_get_modeinfo get_modeinfo;
    ifb_video_driver_draw draw;

    ifb_video_onModeset onModeset;

    listnode_t* self;
}ifb_video_driver_t;

typedef struct ifb_block
{
    ifb_video_info_t* info;

    uint32_t *buffer;
    uint32_t size;
    uint32_t bpp;

    list_t* drivers;
    ifb_video_driver_t* current_driver;
}ifb_block_t;

void init_ifb();

bool isIFB_enabled();
void change_IFB_state(bool state);

void ifb_refresh();

void *ifb_getIFB();

void register_video_driver(ifb_video_driver_t* drv);
void register_modeset_handler(ifb_video_onModeset onModeset);

uint32_t video_get_width();
uint32_t video_get_height();
uint32_t video_get_bpp();

ifb_video_info_t* video_get_modeinfo();

int video_modeset(uint32_t width, uint32_t height, uint32_t bpp);
int video_draw(uint32_t* fb);

#endif /*__IFB_H__*/