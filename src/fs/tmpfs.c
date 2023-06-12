#include "tmpfs.h"
#include "rtc.h"
#include "timer.h"
#include "mount.h"

//Single mountpoint file system

int curr_idx;

#include "logdisk.h"

void init_tmpfs(char* mountpoint)
{
    ldprintf("tmpfs", LOG_DEBUG, "Mounting to %s", mountpoint);
    tmpfs_t* fs = ZALLOC_TYPES(tmpfs_t);
    fs->root = ZALLOC_TYPES(FILE);
    FILE* root = fs->root;

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

    fs->bitmap = zalloc(sizeof(tmpfs_file_t)*TMPFS_MAX_FILES);

    if(mount_list != NULL)
    {
        mount_info_t* m = ZALLOC_TYPES(mount_info_t);
        m->device = "ram";
        m->mountpoint = strdup(mountpoint);
        m->fs_type = FS_TYPE_TMPFS;
        list_push(mount_list, m);
    }

    vfs_mount(mountpoint, fs->root);
}

uint32_t tmpfs_read(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    tmpfs_t* fs = f->device;
    tmpfs_file_t* file = &(fs->bitmap[f->inode_num]);

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
    tmpfs_file_t* file = &(fs->bitmap[f->inode_num]);

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
    tmpfs_t* fs = node->device;
    if(index > (fs->n_files-1))
        return NULL;
    DirectoryEntry* d = ZALLOC_TYPES(DirectoryEntry);
    tmpfs_file_t* f = &(fs->bitmap[index]);
    d->inode_count = f->idx;
    strcpy(d->name, f->self->name);
    ldprintf("tmpfs", LOG_DEBUG, "Got readdir request. Returning : {\"%s\", %d}", f->self->name, f->idx);
    return d;
}

char** tmpfs_listdir(FILE* parent)
{
    tmpfs_t* fs = parent->device;
    register int i = 0;

    char** list = kmalloc(sizeof(char*)*(fs->n_files + 1));
    for(i = 0; i < fs->n_files; i++)
    {
        list[i] = kmalloc(sizeof(char) * 256);
        strcpy(list[i], fs->bitmap[i].self->name);
        serialprintf("%d: %s:%s\n", i, list[i], fs->bitmap[i].self->name);
    }
    list[i] = NULL;
    return list;
}

FILE* tmpfs_finddir(FILE* parent, char* name)
{
    tmpfs_t* fs = parent->device;
    register int i = 0;

    for(i = 0; i < (fs->n_files); i++)
    {
        FILE* f = fs->bitmap[i].self;
        if(strcmp(f->name, name) == 0)
        {
            return f;
        }
    }
    return NULL;
}

void tmpfs_create(FILE* parent, char* name, uint32_t permission)
{
    tmpfs_t* fs = parent->device;
    uint32_t idx = fs->n_files++;
    tmpfs_file_t* files = fs->bitmap;
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
    fs->size += 100;
    serialprintf("[TMPFS] File created \"/tmp/%s\"\n", name);
    
}

void tmpfs_open(FILE* f, uint32_t flags)
{
    f->open_flags = flags;
}

uint32_t tmpfs_getfilesize(FILE* f)
{
    tmpfs_t* fs = f->device;
    uint32_t size = fs->bitmap[f->inode_num].current_size;
    return size;
}

void tmpfs_close(FILE* f)
{
    return;
}

void tmpfs_debug_list(tmpfs_t* fs)
{
    uint32_t nf = fs->n_files;
    register int i = 0;

    for(i = 0; i < nf; i++)
    {
        FILE* f = fs->bitmap[i].self;
        serialprintf("%s ", f->name);
        printf("%s ", f->name);
    }
    printf("\n");
    serialprintf("\n");
    serialprintf("FILES: %d\nSIZE: %d\n", fs->n_files, fs->size);
}
