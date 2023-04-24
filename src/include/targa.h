#ifndef __TARGA_H__
#define __TARGA_H__

#include "system.h"
#include "string.h"
#include "vesa.h"
#include "kheap.h"
#include "vfs.h"

typedef struct targa
{
    uint32_t width;
    uint32_t height;
    uint32_t size;

    uint32_t* img_buffer;
}targa_t;

targa_t* targa_parse(char* filename);
void targa_display(char* name);

#endif /*__TARGA_H__*/