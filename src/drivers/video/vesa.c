#include "vesa.h"

uint32_t current_mode;
int isVesaInitialized = 0;
VBE_MODE_INFO_t current_mode_info = {0};

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

uint32_t rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (0x00ff0000 & (r << 16)) | (0x0000ff00 & (g << 8)) | (0x000000ff & b);
}

void vesa_copy_framebuffer(void* fb)
{
    memcpy(current_mode_info.phys_base, fb, (current_mode_info.pitch * current_mode_info.YResolution));
}

void vesa_change_mode(uint32_t width, uint32_t height, uint32_t bpp)
{
}

void init_vesa(multiboot_info_t *m)
{
    alloc_region(kernel_page_dir, m->vbe_mode_info, m->vbe_mode_info+sizeof(VBE_MODE_INFO_t), 1, 1, 1);
    current_mode_info = *((VBE_MODE_INFO_t*)m->vbe_mode_info);

    void* framebuffer = vesa_getFramebuffer();
    alloc_region(kernel_page_dir, (uint32_t)framebuffer, (uint32_t)(framebuffer + vesa_getXResolution() * vesa_getYResolution() * 4), 1, 1, 1);

    pic_eoi(0x28);

    isVesaInitialized = 1;

    serialprintf("VESA initialized!\n");

    serialprintf("Mode: %x\n", current_mode);
    serialprintf("XResolution: %d\n", current_mode_info.XResolution);
    serialprintf("YResolution: %d\n", current_mode_info.YResolution);
    serialprintf("BitsPerPixel: %d\n", current_mode_info.bpp);
    serialprintf("PhysBasePtr: %x\n", current_mode_info.phys_base);
    serialprintf("Pitch: %d\n", current_mode_info.pitch);
}

bool isVesaInit()
{
    return (isVesaInitialized == 1);
}

VBE_MODE_INFO_t* vesa_get_current_mode()
{
    return &current_mode_info;
}