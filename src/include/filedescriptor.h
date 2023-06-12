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


typedef struct dirent {
	uint32_t d_ino;
	char d_name[256];
} fd_dirent;

#define FD_MAX 512
#define FD_SIZE sizeof(fd_t)

int fd_open(char* file, int flags, int mode);
int fd_close(int file);
int fd_read(int file, char* ptr, int len);
int fd_write(int file, char* ptr, int len);
int fd_lseek(int file, int ptr, int dir);
int fd_ioctl(int file, int req, void* data);
int fstat(int file, seos_stat_t* st);
int fd_readdir(int fd, int index, fd_dirent* dirent);
//uint32_t append_fd(pcb_t* process, FILE* f);

int access(const char* file, int flags);
uint32_t umask(uint32_t mask);
int chmod(char* file, int mode);

void list_descriptors();

#endif /*__FILE_DESCRIPTOR_H__*/