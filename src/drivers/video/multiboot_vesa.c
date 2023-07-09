#include "multiboot_vesa.h"
#include "ifb.h"

uint32_t current_mode;
int isVesaInitialized = 0;
VBE_MODE_INFO_t current_mode_info = {0};

ifb_video_driver_t* vesa_driver = NULL;

void vesa_memcpy24_to_32(uint24_t* dest, uint32_t* src, size_t size)
{
    uint24_t t;
    uint32_t i;
    for(i = 0; i < size; i++)
    {
        t.integer = src[i];
        dest[i] = t;
    }
}

void vesa_memsetRGB(uint8_t * dest, uint32_t rgb, uint32_t count)
{
    if(count % 3 != 0)
        count = count + 3 - (count % 3);
    uint8_t r = rgb & 0x00ff0000;
    uint8_t g = rgb & 0x0000ff00;
    uint8_t b = rgb & 0x000000ff;
    for(int i = 0; i < count; i++)
    {
        *dest++ = r;
        *dest++ = g;
        *dest++ = b;
    }
}

void* vesa_getFramebuffer()
{
    return (void*)current_mode_info.phys_base;
}

int vesa_getXResolution()
{
    return current_mode_info.XResolution;
}

int vesa_getYResolution()
{
    return current_mode_info.YResolution;
}

void vesa_putPixel(uint32_t x, uint32_t y, uint32_t color)
{
    uint32_t pitch = current_mode_info.pitch;
    uint32_t bpp = current_mode_info.bpp;
    uint32_t framebuffer = current_mode_info.phys_base;

    uint32_t pixel_offset = y * pitch + (x * (bpp/8)) + framebuffer;

    ((uint8_t*)pixel_offset)[0] = color & 0x000000ff;
    ((uint8_t*)pixel_offset)[1] = (color & 0x0000ff00) >> 8;
    ((uint8_t*)pixel_offset)[2] = (color & 0x00ff0000) >> 16;
    ((uint8_t*)pixel_offset)[3] = (color & 0xff000000) >> 24;
}

void vesa_copy_framebuffer(void* fb)
{
    memcpy((void*)current_mode_info.phys_base, fb, (current_mode_info.pitch * current_mode_info.YResolution));
}

int vesa_modeset(uint32_t width, uint32_t height, uint32_t bpp)
{
    return 1;
}

int vesa_draw(uint32_t* fb, uint32_t width, uint32_t height)
{
    // if(validate(current_mode_info.phys_base) != 1)
    //     return 1;
    fast_memcpy((void*)current_mode_info.phys_base, fb, (current_mode_info.pitch * current_mode_info.YResolution));
    return 0;
}

ifb_video_info_t* vesa_get_modeinfo()
{
    ifb_video_info_t* info = ZALLOC_TYPES(ifb_video_info_t);
    info->width = current_mode_info.XResolution;
    info->height = current_mode_info.YResolution;
    info->bpp = current_mode_info.bpp;
    info->pitch = current_mode_info.pitch;
    info->size = info->pitch * info->height;
    info->fb = current_mode_info.phys_base;
    info->pix = IFB_PIXELFORMAT_ARGB;

    return info;
}

void init_vesa(struct multiboot_tag_vbe* vbe)
{
    current_mode_info = *((VBE_MODE_INFO_t*)vbe->vbe_mode_info.external_specification);
    current_mode = vbe->vbe_mode;

    uint32_t* framebuffer = current_mode_info.phys_base;
    alloc_region(kernel_page_dir, (uint32_t)framebuffer, (uint32_t)(framebuffer + vesa_getXResolution() * vesa_getYResolution() * 4), 1, 1, 1);

    pic_eoi(0x28);

    serialprintf("VESA initialized!\n");

    serialprintf("Mode: %x\n", current_mode);
    serialprintf("XResolution: %d\n", current_mode_info.XResolution);
    serialprintf("YResolution: %d\n", current_mode_info.YResolution);
    serialprintf("BitsPerPixel: %d\n", current_mode_info.bpp);
    serialprintf("PhysBasePtr: %x\n", current_mode_info.phys_base);
    serialprintf("Pitch: %d\n", current_mode_info.pitch);

    vesa_driver = ZALLOC_TYPES(ifb_video_driver_t);
    strcpy(vesa_driver->name, "multiboot vesa");
    vesa_driver->draw = vesa_draw;
    vesa_driver->get_modeinfo = vesa_get_modeinfo;
    vesa_driver->modeset = 0;
    vesa_driver->flags = 0;

    register_video_driver(vesa_driver);

    isVesaInitialized = 1;
}

bool isVesaInit()
{
    return (isVesaInitialized == 1);
}

VBE_MODE_INFO_t* vesa_get_current_mode()
{
    return &current_mode_info;
}