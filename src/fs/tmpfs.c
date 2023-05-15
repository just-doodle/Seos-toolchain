#include "tmpfs.h"
#include "rtc.h"
#include "timer.h"

//Single mountpoint file system

int curr_idx;

tmpfs_file_t* files;

FILE* root;

void init_tmpfs(char* mountpoint)
{
    tmpfs_t* fs = ZALLOC_TYPES(tmpfs_t);
    root = ZALLOC_TYPES(FILE);

    struct timeval t;
    gettimeofday(&t, NULL);

    strcpy(root->name, "tmpfs");
    root->device = fs;
    root->dev_id = 98;
    root->creation_time = t.tv_sec;

    root->open = tmpfs_open;
    root->close = tmpfs_close;
    root->listdir = tmpfs_listdir;
    root->finddir = tmpfs_finddir;
    root->readdir = tmpfs_readdir;
    root->create = tmpfs_create;

    root->flags = FS_DIRECTORY;

    files = zalloc(sizeof(tmpfs_file_t)*TMPFS_MAX_FILES);
    fs->bitmap = files;

    vfs_mount(mountpoint, root);
}

uint32_t tmpfs_read(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    tmpfs_t* fs = f->device;
    tmpfs_file_t* file = &(files[f->inode_num]);

    if(file == NULL)
        return -1;

    if((offset+size) > file->current_size)
        return -1;

    f->last_accessed = gettimeofday_seconds();
    memcpy(buffer, file->buffer+offset, size);
    return size;
}

uint32_t tmpfs_write(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    tmpfs_t* fs = f->device;
    tmpfs_file_t* file = &(files[f->inode_num]);

    uint32_t size_needed = 0;

    if(file == NULL)
        return -1;

    if(f->open_flags & OPEN_TRUNC)
    {
        fs->size -= file->current_size;
        file->current_size = 0;
    }

    if((offset+size) > file->current_size)
    {
        size_needed = (offset+size) - file->current_size;
        file->current_size += size_needed;
        file->buffer = realloc(file->buffer, file->current_size);
        fs->size += size_needed;
    }

    f->last_accessed = f->last_modified = gettimeofday_seconds();
    memcpy(file->buffer+offset, buffer, size);
    return size;
}

DirectoryEntry* tmpfs_readdir(FILE* node, uint32_t index)
{
    if(node == root)
    {
        DirectoryEntry* d = ZALLOC_TYPES(DirectoryEntry);
        tmpfs_file_t* f = &files[index];
        d->inode_count = f->idx;
        strcpy(d->name, f->self->name);
        return d;
    }
}

char** tmpfs_listdir(FILE* parent)
{
    tmpfs_t* fs = parent->device;
    register int i = 0;

    char** list = kmalloc(sizeof(char*)*(curr_idx + 1));
    for(i = 0; i < curr_idx; i++)
    {
        list[i] = kmalloc(sizeof(char) * 256);
        strcpy(list[i], files[i].self->name);
        serialprintf("%d: %s:%s\n", i, list[i],files[i].self->name);
    }
    list[i] = NULL;
    return list;
}

FILE* tmpfs_finddir(FILE* parent, char* name)
{
    tmpfs_t* fs = parent->device;
    register int i = 0;

    for(i = 0; i < (curr_idx); i++)
    {
        FILE* f = files[i].self;
        if(strcmp(f->name, name) == 0)
        {
            return f;
        }
    }
    return NULL;
}

void tmpfs_create(FILE* parent, char* name, uint32_t permission)
{
    if(parent == root)
    {
        tmpfs_t* fs = root->device;
        uint32_t idx = curr_idx++;

        files[idx].idx = idx;
        files[idx].magic = TMPFS_MAGIC;
        files[idx].current_size = 10;
        files[idx].buffer = zalloc(10);
        files[idx].self = ZALLOC_TYPES(FILE);
        strcpy(files[idx].self->name, name);
        files[idx].self->open = tmpfs_open;
        files[idx].self->close = tmpfs_close;
        files[idx].self->read = tmpfs_read;
        files[idx].self->write = tmpfs_write;
        files[idx].self->get_filesize = tmpfs_getfilesize;
        files[idx].self->inode_num = idx;
        files[idx].self->device = fs;
        files[idx].self->size = 100;
        files[idx].self->flags |= FS_FILE;
        files[idx].self->mask = permission & 0xFFF;
        files[idx].self->creation_time = gettimeofday_seconds();

        fs->size += 10;
        fs->n_files++;

        serialprintf("[TMPFS] File created \"/tmp/%s\"\n", name);
    }
}

void tmpfs_open(FILE* f, uint32_t flags)
{
    f->open_flags = flags;
}

uint32_t tmpfs_getfilesize(FILE* f)
{
    if(f->device == root->device)
    {
        uint32_t size = files[root->inode_num].current_size;
        return size;
    }
    return -1;
}

void tmpfs_close(FILE* f)
{
    return;
}

void tmpfs_debug_list()
{
    tmpfs_t* fs = root->device;
    uint32_t nf = fs->n_files;
    register int i = 0;

    for(i = 0; i < curr_idx; i++)
    {
        FILE* f = files[i].self;
        serialprintf("%s ", f->name);
        printf("%s ", f->name);
    }
    printf("\n");
    serialprintf("\n");
    serialprintf("FILES: %d\nSIZE: %d\n", fs->n_files, fs->size);
}
