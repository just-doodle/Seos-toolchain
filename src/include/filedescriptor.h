#ifndef __FILE_DESCRIPTOR_H__
#define __FILE_DESCRIPTOR_H__

#include "system.h"
#include "vfs.h"
#include "string.h"
#include "kheap.h"
#include "stat.h"

#ifndef SEEK_SET
#define	SEEK_SET	0	/* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define	SEEK_CUR	1	/* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define	SEEK_END	2	/* set file offset to EOF plus offset */
#endif

typedef struct file_descriptor_struct
{
    int index;
    char *path;
    FILE* file;
    int mode;
    int flags;
    uint32_t seek;
    uint32_t size;
}fd_t;

#define FD_MAX 512
#define FD_SIZE sizeof(fd_t)

int fd_open(char* file, int flags, int mode);
int fd_close(int file);
int fd_read(int file, char* ptr, int len);
int fd_write(int file, char* ptr, int len);
int fd_lseek(int file, int ptr, int dir);
int fstat(int file, seos_stat_t* st);

void list_descriptors();

#endif /*__FILE_DESCRIPTOR_H__*/