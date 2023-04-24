#ifndef __STDOUT_H__
#define __STDOUT_H__

#include "system.h"
#include "printf.h"
#include "kheap.h"
#include "devfs.h"
#include "vfs.h"

void init_stdout();

uint32_t stdout_write(FILE* f, uint32_t off, size_t sz, char* buf);
void stdout_open(FILE* f, uint32_t flags);
void stdout_close(FILE* f);

uint32_t stdin_read(FILE* f, uint32_t off, size_t sz, char* buf);
void stdin_open(FILE* f, uint32_t flags);
void stdin_close(FILE* f);
void stdin_keybuff(uint8_t scancode);

uint32_t stderr_write(FILE* f, uint32_t off, size_t sz, char* buf);
void stderr_open(FILE* f, uint32_t flags);
void stderr_close(FILE* f);

#endif /*__STDOUT_H__*/