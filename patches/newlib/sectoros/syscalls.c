/* note these headers are all provided by newlib - you don't need to provide them */
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/times.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>

#include <bits/dirent.h>

#include "user_syscall.h"
#include "sys/termios.h"
#include "sys/utsname.h"

typedef struct stat_struct
{
    uint64_t dev_id;
    unsigned long ino;
    uint32_t mode;
    uint32_t nlink;
    uint32_t uid;
    uint32_t gid;
    long size;

    uint32_t last_modified;
    uint32_t last_accessed;
    uint32_t creation_time;
    long blocks;
    long int blksize;
}seos_stat_t;

//char *__env[1] = { 0 };
//char **environ = __env;

void _exit()
{
    SYSCALL_EXIT(0);
}

int close(int file)
{
    int ret = 0;
    SYSCALL_CLOSE(file, ret);
    return ret;
}

int execve(char *name, char **argv, char **env)
{
    SYSCALL_EXECVE(name, argv, env);
    return 0;
}

int fork()
{
    errno = EAGAIN;
    return -1;
}

int fstat(int file, struct stat *st)
{
    int ret = 0;
    seos_stat_t sta;
    SYSCALL_FSTAT(file, &sta, ret);
    st->st_dev = sta.dev_id;
    st->st_ino = sta.ino;
    st->st_blocks = sta.blocks;
    st->st_gid = sta.gid;
    st->st_uid = sta.uid;
    st->st_rdev = 0;
    st->st_mode = sta.mode;
    st->st_size = sta.size;
    st->st_nlink = sta.nlink;
    st->st_atime = sta.last_accessed;
    st->st_ctime = sta.creation_time;
    st->st_mtime = sta.last_modified;
    return ret;
}

int getpid()
{
    int pid = 0;
    SYSCALL_GETPID(pid);
    return pid;
}

int isatty(int file)
{
    return (file < 3);
}

char *ttyname(int fd)
{
	errno = ENOTTY;
	return NULL;
}

long sysconf(int name)
{
	switch (name)
    {
		case 8:
			return 4096;
		case 11:
			return 10000;
		default:
			return -1;
	}
}

char* getwd(char* buf)
{
    char *wd = 0;
    SYSCALL_GETCWD(wd);
    strcpy(buf, wd);
    return buf;
}

char* getcwd(char* buf, uint32_t size)
{
    char* wd = 0;
    SYSCALL_GETCWD(wd);
    if(size > strlen(wd))
    {
        errno = EINVAL;
        return NULL;
    }
    strncpy(buf, wd, size);
    return buf;
}

int chdir(const char* path)
{
    SYSCALL_CHDIR(path);
    return 0;
}

int ioctl(int fd, int req, void* argp)
{
    int ret = 0;
    SYSCALL_IOCTL(fd, ret, req, argp);
    return ret;
}

speed_t cfgetispeed(const struct termios * tio)
{
	return 0;
}

speed_t cfgetospeed(const struct termios * tio)
{
	return 0;
}

int cfsetispeed(struct termios * tio, speed_t speed)
{
	return 0;
}

int cfsetospeed(struct termios * tio, speed_t speed)
{
	return 0;
}

int tcdrain(int i)
{
	return 0;
}

int tcflow(int a, int b)
{
	return 0;
}

int tcflush(int a, int b)
{
	return 0;
}

int tcgetattr(int fd, struct termios * tio)
{
	return 0;
}

pid_t tcgetsid(int fd)
{
	return getpid();
}

int tcsendbreak(int a, int b)
{
	return 0;
}

int tcsetattr(int fd, int actions, struct termios * tio)
{
	return 0;
}

int tcsetpgrp(int fd, pid_t pgrp)
{
	return 0;
}

pid_t tcgetpgrp(int fd)
{
	return -1;
}

int kill(int pid, int sig)
{
    int ret = 0;
    SYSCALL_KILL(pid, sig, ret);
    return ret;
}

int link(char *old, char *new)
{
    errno = EMLINK;
    return -1;
}

uint32_t getuid()
{
    uint32_t ret = 0;
    SYSCALL_GETUID(ret)
    return ret;
}

int setuid(uint32_t new_uid)
{
    int ret = 0;
    SYSCALL_SETUID(ret, new_uid)
    return ret;
}

uint32_t geteuid()
{
    return getuid();
}

uint32_t getgid()
{
    return 0;
}

uint32_t getegid()
{
    return getgid();
}

long pathconf(char *path, int name)
{
    return 0;
}

int utime(const char *filename, const struct utimbuf *times)
{
    return 0;
}

int chown(const char *path, uid_t owner, gid_t group)
{
    return 0;
}

int rmdir(const char *pathname)
{
    return 0;
}

int getgroups(int size, gid_t list[])
{
    return 0;
}

pid_t getppid()
{
	return 0;
}

int fpathconf(int file, int name)
{
    return 0;
}

int fcntl(int fd, int cmd, ...)
{
	if (cmd == F_GETFD || cmd == F_SETFD)
    {
		return 0;
	}
	return -1;
}

void sync()
{
}

int setgid(uint32_t newgid)
{
    return -1;
}

int uname(struct utsname* name)
{
    int ret = 0;
    SYSCALL_UNAME(name);
    return ret;
}

mode_t umask(mode_t new_mask)
{
    mode_t ret = 0;
    SYSCALL_UMASK(new_mask, ret)
    return ret;
}

int access(const char* file, int flags)
{
    int ret = 0;
    SYSCALL_ACCESS(file, flags, ret)
    return ret;
}

int chmod(const char* file, mode_t mode)
{
    int ret = 0;
    SYSCALL_CHMOD(ret, file, mode);
    return ret;
}

int mount(const char* source, const char* target, const char* fstype, int modeFlags, char* fsargs)
{
    int ret = 0;
    SYSCALL_MOUNT(source, target, fstype, ret)
    return ret;
}

int reboot(int cmd)
{
    int ret = 0;
    SYSCALL_REBOOT(ret);
    return ret;
}


int lseek(int file, int ptr, int dir)
{
    int ret = 0;
    SYSCALL_LSEEK(file, ptr, dir, ret);
    return ret;
}

int open(const char *name, int flags, ...)
{
    int ret = 0;
    SYSCALL_OPEN(name, flags, 0, ret);
    return ret;
}

int read(int file, char *ptr, int len)
{
    int ret = 0;
    SYSCALL_READ(file, ptr, len, ret);
    return ret;
}

char* sbrk(int incr)
{
    char* ret = NULL;
    SYSCALL_SBRK(incr, ret);
    return ret;
}

int stat(const char *file, struct stat *st)
{
    int ret = 0;
    seos_stat_t sta;
    SYSCALL_STAT(file, &sta, ret);
    st->st_dev = sta.dev_id;
    st->st_ino = sta.ino;
    st->st_blocks = sta.blocks;
    st->st_gid = sta.gid;
    st->st_uid = sta.uid;
    st->st_rdev = 0;
    st->st_mode = sta.mode;
    st->st_size = sta.size;
    st->st_nlink = sta.nlink;
    st->st_atime = sta.last_accessed;
    st->st_ctime = sta.creation_time;
    st->st_mtime = sta.last_modified;
    return ret;
}

DIR* opendir(const char * dirname)
{
	int fd = open(dirname, 0);
	if (fd == -1)
    {
		return NULL;
	}

	DIR * dir = malloc(sizeof(DIR));
	dir->fd = fd;
	dir->cur_entry = -1;
	return dir;
}

int closedir(DIR* dir)
{
	if (dir && (dir->fd != -1))
    {
		return close(dir->fd);
	} else {
		return -1;
	}
}

struct dirent* readdir(DIR* dirp)
{
	static struct dirent ent;
    memset(&ent, 0, sizeof(struct dirent));

	int ret = 0;
    SYSCALL_READDIR(dirp->fd, ++dirp->cur_entry, &ent, ret)
	if (ret != 0)
    {
		memset(&ent, 0, sizeof(struct dirent));
		return NULL;
	}

	return &ent;
}

clock_t times(struct tms *buf)
{
    return -1;
}

unsigned int alarm(unsigned int seconds)
{
    return 0;
}

int mkdir(const char* file, mode_t mode)
{
    SYSCALL_MKDIR(file, mode)
    return 0;
}

int unlink(char *name)
{
    FILE* f = fopen(name, "r");
    if(f != NULL)
    {
        fclose(f);
        SYSCALL_UNLINK(name);
    }
    return -1; 
}

int wait(int *status)
{
    errno = ECHILD;
    return -1;
}

int write(int file, char *ptr, int len)
{
    int ret = 0;
    SYSCALL_WRITE(file, ptr, len, ret);
    return ret;
}

int gettimeofday(struct timeval *p, void *z)
{
    SYSCALL_GETTIMEOFDAY(p, z)
    return 0;
}