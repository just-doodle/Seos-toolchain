#ifndef __CHARBUFFER_H__
#define __CHARBUFFER_H__

#include "system.h"
#include "kheap.h"
#include "printf.h"
#include "string.h"
#include "spinlock.h"

typedef struct charbuffer_struct
{
    char* buffer;

    uint32_t size;

    uint32_t rw_ptr;

    uint32_t lock;
}charbuffer_t;

charbuffer_t* create_charbuffer(uint32_t size);

void charbuffer_push(charbuffer_t* buf, char val);
char charbuffer_pop(charbuffer_t* buf);

void charbuffer_read(charbuffer_t* buf, uint32_t offset, uint32_t size, char* buffer);
void charbuffer_write(charbuffer_t* buf, uint32_t offset, uint32_t size, char* buffer);

void charbuffer_clean(charbuffer_t* buf);
void charbuffer_wait(charbuffer_t* buf, uint32_t size);
void charbuffer_waitchar(charbuffer_t* buf, char c);

void charbuffer_dump(charbuffer_t* buf, char* buffer);

#endif /*__CHARBUFFER_H__*/