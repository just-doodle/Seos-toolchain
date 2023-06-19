#include "filedescriptor.h"
#include "process.h"

fd_t* get_closed_fd()
{
    fd_t* f;
    for(int i = 0; i < current_process->fd_num; i++)
    {
        f = &(current_process->fds[i]);
        if(f->index == -1)
        {
            serialprintf("[FDM] Got closed fd #%d\n", i);
            f->index = i;
            return f;
        }
    }
    return NULL;
}

fd_t* get_closed_fd_from_process(pcb_t* p)
{
    fd_t* f;
    for(int i = 0; i < p->fd_num; i++)
    {
        f = &(p->fds[i]);
        if(f->index == -1)
        {
            serialprintf("[FDM] Got closed fd #%d\n", i);
            f->index = i;
            return f;
        }
    }
    return NULL;
}

fd_t* get_fd(int idx)
{
    for(int i = 0; i < current_process->fd_num; i++)
    {
        fd_t* f = &(current_process->fds[i]);
        if(f->index == idx)
        {
            return f;
        }
    }
    return NULL;
}

int fd_open(char* file, int flags, int mode)
{
    if(current_process->fd_num > FD_MAX)
    {
        return -1;
    }
    if(virt2phys(kernel_page_dir, file) == NULL)
    {
        if(virt2phys(current_process->page_dir, file) == NULL)
            return -1;
    }
    if(strlen(file) == 0)
    {
        return -1;
    }
    char* fsave = zalloc(strlen(file) + strlen(p__cwd));
    if(file[0] == '.')
    {
        strcpy(fsave, p__cwd);
        int o = strlen(p__cwd);
        memcpy(fsave+o, file+1, strlen(file));
    }
    else
    {
        strcpy(fsave, file);
    }
    FILE* f = file_open(fsave, flags);
    serialprintf("[FDM] Trying to open %s: %d\n", fsave, flags);
    if(f == NULL)
    {
        // if((flags & OPEN_RDWR) == OPEN_RDWR || (flags & OPEN_WRONLY) == OPEN_WRONLY)
        // {
        //     vfs_create(file, current_process->umask);
        //     FILE* f = file_open(fsave, flags);
        //     if(f == NULL)
        //         return -1;
        // }
        // else
        // {
            serialprintf("[FDM] No file %s\n", fsave);
            return -1;
        // }
    }

    fd_t* fd = get_closed_fd();
    if(fd == NULL)
    {
        fd = &(current_process->fds[current_process->fd_num]);
        fd->index = current_process->fd_num;
        current_process->fd_num++;
    }
    fd->file = f;
    fd->size = f->size;
    if(f->get_filesize)
        fd->size = vfs_getFileSize(fd->file);
    fd->mode = mode;
    fd->seek = 0;
    fd->flags = flags;
    fd->path = strdup(fsave);
    free(fsave);
    serialprintf("[FDM] Opening file %s as file descriptor for process #%d:%d\n", fd->file->name, current_process->pid, fd->index);
    return fd->index;
}

int fd_close(int file)
{
    fd_t* f = get_fd(file);
    if(f == NULL)
        return -1;
    serialprintf("[FDM] Closing fd:#%d\n", f->index);
    vfs_close(f->file);
    free(f->path);
    memset(f, 0, FD_SIZE);
    f->index = -1;
    return 0;
}

int fd_read(int file, char* ptr, int len)
{
    fd_t* f = get_fd(file);
    if(f == NULL)
        return -1;
    memset(ptr, 0, len);
    if(f->seek > f->size)
    {
        return 0;
    }
    if(len > (f->size - f->seek))
        len = (f->size - f->seek);
    vfs_read(f->file, f->seek, len, ptr);
    f->seek += len;
    return len;
}

int fd_write(int file, char* ptr, int len)
{
    fd_t* f = get_fd(file);
    if(f == NULL)
        return -1;
    if(f == NULL)
    {
        serialprintf("[FDM] FD#%d is not in FDTABLE.\n", file);
        return -1;
    }
    if(f->file->write == NULL)
    {
        serialprintf("[FDM] Writing not supported for FD#%d: %s:%s.\n", f->index, f->path, f->file->name);
        return -1;
    }
    vfs_write(f->file, f->seek, len, ptr);
    f->seek += len;
    return len;
}

int fd_lseek(int file, int off, int dir)
{
    fd_t* f = get_fd(file);
    if(f == NULL)
        return -1;
    switch(dir)
    {
    case SEEK_SET:
        f->seek = off;
        break;
    case SEEK_CUR:
        f->seek += off;
        break;
    case SEEK_END:
        f->seek = f->size + off;
        break;
    };
    return f->seek;
}

int fstat(int file, seos_stat_t* st)
{
    fd_t* f = get_fd(file);
    if(f == NULL)
        return -1;
    if(strlen(f->path) == 0)
    {
        uint32_t sz = vfs_getFileSize(f);
        st->dev_id = f->file->dev_id;
        st->ino = f->file->inode_num;
        st->gid = f->file->gid;
        st->uid = f->file->uid;
        st->creation_time = f->file->creation_time;
        st->last_accessed = f->file->last_accessed;
        st->last_modified = f->file->last_modified;
        st->mode = f->file->mask;
        st->nlink = f->file->nlink;
        st->blocks = (sz / 512);
        st->blksize = 512;
        st->size = (long)sz;
        return 0;
    }
    int ret = stat(f->path, st);
    return ret;
}

int fd_readdir(int fd, int index, fd_dirent* dirent)
{
    fd_t* f = get_fd(fd);
    if(f == NULL)
        return -1;
    serialprintf("[FDM] READDIR %s\n", f->path);
    DirectoryEntry *dir = vfs_readdir(f->file, index);
    if(dir == NULL)
        return -1;
    memcpy(dirent, dir, sizeof(fd_dirent));
    free(dir);
    return 0;
}

void list_descriptors()
{
    for(int i = 0; i < current_process->fd_num; i++)
    {
        fd_t* f = &current_process->fds[i];
        printf("%d: %s [%d:0x%04x]\n", f->index, f->file->name, f->size, f->seek);
    }
}

int copy_fds(pcb_t* dst, pcb_t* src)
{
    if(dst == NULL || src == NULL)
        return -1;

    for(int i = 0; i < src->fd_num; i++)
    {
        memcpy(&(dst->fds[i]), &(src->fds[i]), sizeof(fd_t));
    }
    dst->fd_num = src->fd_num;
    return 0;
}

int fd_ioctl(int fd, int req, void* data)
{
    fd_t* f = get_fd(fd);
    return vfs_ioctl(f->file, req, data);
}

uint32_t umask(uint32_t mask)
{
    uint32_t pmask = current_process->umask;
    current_process->umask = mask & 777;
    return pmask;
}

int chmod(char* file, int mode)
{
    if(validate(file) != 1)
        return -1;

    FILE* f = file_open(file, OPEN_RDONLY);
    if(f == NULL)
        return -1;
    
    vfs_chmod(f, mode);
    vfs_close(f);
    return 0;
}

int access(const char* file, int flags)
{
    if(validate(file) != 1)
        return -1;

    FILE* f = file_open(file, flags);
    if(f == NULL)
        return -1;

    vfs_close(f);
    return 0;
}

uint32_t append_fd(pcb_t* process, FILE* f)
{
    if(validate(process) != 1)
        return -1;
    if(validate(f) != 1)
        return -1;

    fd_t* fd = get_closed_fd_from_process(process);
    if(fd == NULL)
    {
        fd = &(process->fds[process->fd_num]);
        fd->index = process->fd_num;
        process->fd_num++;
    }

    fd->file = f;
    fd->flags = f->open_flags;
    fd->path = "\0";
    fd->seek = 0;
    fd->size = vfs_getFileSize(f);
    
    return fd->index;
}

