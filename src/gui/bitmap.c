#include "bitmap.h"

bitmap_t * bitmap_create(char * filename)
{
    bitmap_t * ret = kmalloc(sizeof(bitmap_t));

    vfs_node * file = file_open(filename, 0);

    if(!file)
    {
        return NULL;
    }

    uint32_t size = vfs_getFileSize(file);
    void * buf = kmalloc(size);
    vfs_read(file, 0, size, buf);

    serialprintf("LOAD\n");

    bitmap_file_header_t * h = buf;
    unsigned int offset = h->offset;

    bitmap_info_header_t * info = buf + sizeof(bitmap_file_header_t);

    ret->width = info->biWidth;
    ret->height = info->biHeight;
    ret->image_bytes= (void*)((unsigned int)buf + offset);
    ret->buffer = buf;
    ret->total_size= size;
    ret->bpp = info->biBitCount;
    vfs_close(file);
    return ret;
}

void bitmap_display(char* file)
{
    FILE* f = file_open(file, 0);
    if(!f)
    {

        serialprintf("Could not open file %s\n", file);
        return;
    }
    char * buf = kmalloc(vfs_getFileSize(f));
    vfs_read(f, 0, vfs_getFileSize(f), buf);

    bitmap_t * b = bitmap_create(file);

    if(!b)
    {
        return;
    }

    if(b->height > vesa_getYResolution() || b->width > vesa_getXResolution())
    {
        serialprintf("Bitmap is too big! %dx%dx%d\n", b->width, b->height, b->bpp);
        vesa_change_mode(b->width, b->height, b->bpp);
    }

    if(b->height < vesa_getYResolution() || b->width < vesa_getXResolution())
    {
        vesa_change_mode(b->width, b->height, b->bpp);
    }

    uint32_t* framebuffer = (uint32_t*)vesa_getFramebuffer();
    bitmap_to_framebuffer2(b, framebuffer);
}

void bitmap_to_framebuffer(bitmap_t * bmp, uint32_t * frame_buffer)
{
    if(!bmp) return;
    uint8_t * image = bmp->image_bytes;
    int j = 0;
    for(int i = 0; i < bmp->height; i++)
    {
        char * image_row = image + i * bmp->width * 3;
        uint32_t * framebuffer_row = (void*)frame_buffer + (bmp->height - 1 - i) * bmp->width * 4;
        j = 0;
        for(int k = 0; k < bmp->width; k++) {
            uint32_t b = image_row[j++] & 0xff;
            uint32_t g = image_row[j++] & 0xff;
            uint32_t r = image_row[j++] & 0xff;
            uint32_t rgb = ((r << 16) | (g << 8) | (b)) & 0x00ffffff;
            rgb = rgb | 0xff000000;
            framebuffer_row[k] = rgb;
        }
    }
}
void bitmap_to_framebuffer2(bitmap_t * bmp, uint32_t * frame_buffer) 
{
    if(!bmp) return;
    uint8_t * image = bmp->image_bytes;
    int j = 0;
    for(int i = 0; i < bmp->height; i++)
    {
        char * image_row = image + i * bmp->width * 4;
        uint32_t * framebuffer_row = (void*)frame_buffer + (bmp->height - 1 - i) * bmp->width * 4;
        j = 0;
        for(int k = 0; k < bmp->width; k++)
        {
            uint32_t b = image_row[j++] & 0xff;
            uint32_t g = image_row[j++] & 0xff;
            uint32_t r = image_row[j++] & 0xff;
            uint32_t a = image_row[j++] & 0xff;
            uint32_t rgba = ((a << 24) | (r << 16) | (g << 8) | (b));
            framebuffer_row[k] = rgba;
        }
    }
}