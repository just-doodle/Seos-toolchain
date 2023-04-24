#ifndef __BITMAP_H__
#define __BITMAP_H__

#include "system.h"
#include "vfs.h"
#include "string.h"
#include "vesa.h"
#include "kheap.h"

typedef struct BITMAP_FILE_HEADER
{
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} __attribute__((packed)) bitmap_file_header_t;

typedef struct BITMAP_INFO_HEADER
{
    uint32_t biSize;
    long biWidth;
    long biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    long biXPelsPerMeter;
    long biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} __attribute__((packed)) bitmap_info_header_t;

typedef struct bitmap
{
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    char* image_bytes;
    char* buffer;
    uint32_t total_size;
}bitmap_t;

typedef struct palette
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
}palette_t;

bitmap_t * bitmap_create(char * filename);

void bitmap_display(char* file);

void bitmap_to_framebuffer(bitmap_t * bmp, uint32_t * frame_buffer);
void bitmap_to_framebuffer2(bitmap_t * bmp, uint32_t * frame_buffer);


#endif /*__BITMAP_H__*/