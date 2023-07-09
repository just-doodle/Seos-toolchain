#include "multiboot2_tags.h"

multiboot2_tag_ptr_t* tag_ptr = NULL;

int load_multiboot2_tags(void* tags)
{
    if(validate(tags) != 1)
        return 1;

    uint32_t tsz = *((uint32_t*)tags);
    if(tsz == 0)
        return 1;

    tag_ptr = ZALLOC_TYPES(multiboot2_tag_ptr_t);
    tag_ptr->tags_ptr = zalloc(tsz);
    tag_ptr->size = tsz;

    memcpy(tag_ptr->tags_ptr, tags, tsz);

    return 0;
}

int multiboot2_tag_is_available(uint32_t tag_type)
{
    if(validate(tag_ptr) != 1)
        return 0;

    if(validate(tag_ptr->tags_ptr) != 1)
        return 0;

    if(tag_ptr->size == 0)
        return 0;
    
    void* tptr = tag_ptr->tags_ptr;
    for(struct multiboot_tag* tag = (struct multiboot_tag *) ((size_t) (tptr + 8)); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7)))
    {
        if(validate(tag) != 1)
            continue;
        if(tag->type == tag_type)
            return 1;
    }

    return 0;
}

struct multiboot_tag* get_tag_ptr(uint32_t tag_type)
{
    if(validate(tag_ptr) != 1)
        return NULL;

    if(validate(tag_ptr->tags_ptr) != 1)
        return NULL;

    if(tag_ptr->size == 0)
        return NULL;
    
    void* tptr = tag_ptr->tags_ptr;
    for(struct multiboot_tag* tag = (struct multiboot_tag *) ((size_t) (tptr + 8)); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7)))
    {
        if(tag->type == tag_type)
            return tag;
    }

    return NULL;
}