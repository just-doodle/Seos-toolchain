#ifndef __PORTDEV_H__
#define __PORTDEV_H__

#include "system.h"
#include "string.h"
#include "vfs.h"
#include "devfs.h"
#include "kheap.h"

void init_portDev();

uint32_t portdev_read(FILE* f, uint32_t offset, uint32_t size, char* buffer);
uint32_t portdev_write(FILE* f, uint32_t offset, uint32_t size, char* buffer);

void portdev_open(FILE* f, uint32_t flags);
void portdev_close(FILE* f);

#endif /*__PORTDEV_H__*/