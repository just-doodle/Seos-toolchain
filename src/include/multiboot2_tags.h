#ifndef __MULTIBOOT2_TAGS_H__
#define __MULTIBOOT2_TAGS_H__

#include "system.h"
#include "multiboot2.h"
#include "kheap.h"
#include "paging.h"

typedef struct multiboot2_tag_ptr_struct
{
    uint32_t* tags_ptr;
    uint32_t size;
}multiboot2_tag_ptr_t;

int load_multiboot2_tags(void* tag_ptr);
int multiboot2_tag_is_available(uint32_t tag_num);
struct multiboot_tag* get_tag_ptr(uint32_t tag_num);

#endif /*__MULTIBOOT2_TAGS_H__*/