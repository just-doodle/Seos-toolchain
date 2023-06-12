#ifndef __NULLDEV_H__
#define __NULLDEV_H__

#include "system.h"
#include "string.h"
#include "kheap.h"
#include "vfs.h"
#include "devfs.h"

void nulldev_open(FILE* f, uint32_t flags);
void nulldev_close(FILE* f);
uint32_t nulldev_read(FILE* f, uint32_t offset, uint32_t size, char* buffer);
uint32_t nulldev_write(FILE* f, uint32_t offset, uint32_t size, char* buffer);

void zerodev_open(FILE* f, uint32_t flags);
void zerodev_close(FILE* f);
uint32_t zerodev_read(FILE* f, uint32_t offset, uint32_t size, char* buffer);
uint32_t zerodev_write(FILE* f, uint32_t offset, uint32_t size, char* buffer);

void random_open(FILE* f, uint32_t flags);
void random_close(FILE* f);
uint32_t random_read(FILE* f, uint32_t offset, uint32_t size, char* buffer);
uint32_t random_write(FILE* f, uint32_t offset, uint32_t size, char* buffer);

void init_nulldev();

#endif /*__NULLDEV_H__*/