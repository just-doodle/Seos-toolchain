#include "charbuffer.h"
#include "vfs.h"

charbuffer_t* create_charbuffer(uint32_t size)
{
    charbuffer_t* c = ZALLOC_TYPES(charbuffer_t);
    c->buffer = zalloc(size);
    c->size = size;
    c->rw_ptr = 0;
    return c;
}

void charbuffer_push(charbuffer_t* b, char val)
{
    if(b == NULL)
        return;
    if(val == 0)
        return;
    spinlock_lock(&b->lock);
    if(b->rw_ptr > b->size)
        b->rw_ptr = 0;
    b->buffer[b->rw_ptr++] = val;
    spinlock_unlock(&b->lock);
}

char charbuffer_pop(charbuffer_t* b)
{
    if(b == NULL)
        return;
    spinlock_lock(&b->lock);
    char ret = b->buffer[b->rw_ptr];
    if(b->rw_ptr != 0)
        b->rw_ptr--;
    spinlock_unlock(&b->lock);
    return ret;
}

void charbuffer_read(charbuffer_t* buf, uint32_t offset, uint32_t size, char* buffer)
{
    if(buf == NULL)
        return;
    spinlock_lock(&buf->lock);
    if(offset+size > buf->size)
    {
        if(buffer != NULL);
            if(size % 32 == 0)
                memsetdw(buffer, size/32, 0);
            else
                memset(buffer, size, 0);
        
        spinlock_unlock(&buf->lock);
        return;
    }

    if(buffer == NULL)
    {
        spinlock_unlock(&buf->lock);
        return;
    }

    memcpy(buffer, buf->buffer+offset, size);
    spinlock_unlock(&buf->lock);
}

void charbuffer_write(charbuffer_t* buf, uint32_t offset, uint32_t size, char* buffer)
{
    if(buf == NULL)
        return;
    spinlock_lock(&buf->lock);
    if(offset+size > buf->size)
    {
        spinlock_unlock(&buf->lock);
        return;
    }

    if(buffer == NULL)
    {
        spinlock_unlock(&buf->lock);
        return;
    }

    memcpy(buf->buffer+offset, buffer, size);
    spinlock_unlock(&buf->lock);
}

void charbuffer_clean(charbuffer_t* b)
{
    if(b == NULL)
        return;
    spinlock_lock(&b->lock);
    memset(b->buffer, 0, b->size);
    b->rw_ptr = 0;
    spinlock_unlock(&b->lock);
}

void charbuffer_wait(charbuffer_t* b, uint32_t size)
{
    if(b == NULL)
        return;
    uint32_t oldrw_ptr = b->rw_ptr+size;
    while(b->rw_ptr != oldrw_ptr);
}

void charbuffer_waitchar(charbuffer_t* b, char c)
{
    if(b == NULL)
        return;
    asm("sti");
    while(b->buffer[b->rw_ptr-1] != c)
    {
        asm("pause");
    }
}

void charbuffer_dump(charbuffer_t* buf, char* buffer)
{
    if(buf == NULL || buffer == NULL)
        return;

    memcpy(buffer, buf->buffer, buf->rw_ptr);
    buffer[buf->rw_ptr-1] = 0;
    charbuffer_clean(buf);
}

void charbuffer_dump_noclean(charbuffer_t* buf, char* buffer)
{
    if(buf == NULL || buffer == NULL)
        return;

    memcpy(buffer, buf->buffer, buf->rw_ptr);
    buffer[buf->rw_ptr-1] = 0;
}

void charbuffer_dump_to_file(charbuffer_t* buf, char* file)
{
    if(buf == NULL || file == NULL)
        return;

    uint32_t sz = buf->rw_ptr;
    FILE* f = file_open(file, OPEN_RDWR);
    vfs_write(f, 0, sz, buf->buffer);
    vfs_close(f);
}