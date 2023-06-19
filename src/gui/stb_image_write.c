#include "system.h"
#include "string.h"
#include "debug.h"
#include "kheap.h"

#define STBIW_MALLOC(p) kmalloc(p)
#define STBIW_FREE(p) kfree(p)
#define STBIW_REALLOC(p, s) krealloc(p, s)
#define STBI_WRITE_NO_STDIO
#define STBIW_MEMMOVE(a, b, s) memcpy(a, b, s)
#define STBIW_ASSERT(x) ASSERT(x)

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"