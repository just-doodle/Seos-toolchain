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
            return;
    }
    if(strlen(file) == 0)
    {
        return -1;
    }
    FILE* f = file_open(file, mode);
    serialprintf("[FDM] Trying to open %s\n", file);
    if(f == NULL)
    {
        serialprintf("[FDM] No file %s\n", file);
        return -1;
    }

    fd_t* fd = get_closed_fd();
    if(fd == NULL)
    {
        fd = &(current_process->fds[current_process->fd_num]);
        fd->index = current_process->fd_num;
        current_process->fd_num++;
    }
    fd->file = f;
    fd->size = vfs_getFileSize(fd->file);
    fd->mode = mode;
    fd->seek = 0;
    fd->flags = flags;
    fd->path = strdup(file);
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
        serialprintf("[FDM] Writing not supported for FD#%d: %s.\n", f->index, f->path);
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
    int ret = stat(f->path, st);
    return ret;
}

int fd_readdir(int fd, int index, fd_dirent* dirent)
{
    fd_t* f = get_fd(fd);
    if(f == NULL)
        return -1;
    serialprintf("[FDM] READDIR\n");
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