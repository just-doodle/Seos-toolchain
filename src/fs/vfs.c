#include "vfs.h"
#include "sorfs.h"
#include "logdisk.h"
#include "process.h"
#include "kernelfs.h"

tree_t *vfs_tree;
vfs_node* vfs_root;

list_t* filesystems = NULL;

uint32_t vfs_getFileSize(vfs_node* node)
{
    if (node && node->get_filesize)
    {
        return node->get_filesize(node);
    }
    return 0;
}

void vfs_db_listdir(char *name)
{
    vfs_node *n = file_open(name, 0);
    if (!n)
    {
        ldprintf("vfs->db_listdir", LOG_ERR, "Tried to list a directory that does not exist. %s", name);
        return;
    }
    if (!n->listdir)
        return;
    char **files = n->listdir(n);
    char **save = files;
    while (*files)
    {
        printf("%s ", *files);
        kfree(*files);
        files++;
    }
    kfree(save);
    printf("\n");
}

char** vfs_listdir(char* path)
{
    vfs_node *n = file_open(path, 0);
    if (!n)
    {
        ldprintf("vfs->listdir", LOG_ERR, "Tried to list a directory that does not exist. %s", path);
        return NULL;
    }
    if (!n->listdir)
        return NULL;
    char **files = n->listdir(n);
    return files;
}

void print_vfstree_recur(treenode_t *node, int parent_offset)
{
    if (!node) return;
    char * tmp = kmalloc(512);
    int len = 0;
    memset(tmp, 0, 512);
    for (unsigned int i = 0; i < parent_offset; ++i)
    {
        strcat(tmp, " ");
    }
    //char * curr = tmp + strlen(tmp);
    vfs_entry * fnode = (vfs_entry *)node->data;
    if (fnode->file)
    {
        printf("%s%s(0x%x, %s)", tmp, fnode->name, (unsigned int)fnode->file, fnode->file->name);
    }
    else
    {
        printf("%s%s(empty)", tmp, fnode->name);
    }
    printf("%s\n", tmp);
    len = strlen(fnode->name);
    kfree(tmp);
    foreach(child, node->children)
    {
        print_vfstree_recur(child->val, parent_offset + len + 1);
    }
}

void print_vfs_tree()
{
    print_vfstree_recur(vfs_tree->root, 0);
}

void vfs_register_fs(char *name, vfs_mount_callback callm, fsinfo_test test, uint32_t flags)
{
    if(validate(filesystems) != 1)
        return;
    vfs_fsinfo_t* fs = ZALLOC_TYPES(vfs_fsinfo_t);
    fs->name = name;
    fs->mount = callm;
    fs->uid = rand();
    fs->test = test;
    fs->flags = flags;
    list_push(filesystems, fs);

    char* fs_f = zalloc((list_size(filesystems)*32)+2048);
    foreach(k, filesystems)
    {
        vfs_fsinfo_t* fsi = k->val;
        if(!(fsi->flags & FSINFO_FLAGS_NO_KERNELFS_ENTRY))
        {
            if((fsi->flags & FSINFO_FLAGS_NODEV))
                sprintf((fs_f+strlen(fs_f)), "nodev   %s\n", fsi->name);
            else
                sprintf((fs_f+strlen(fs_f)), "        %s\n", fsi->name);
        }
    }
    kernelfs_addcharf("/proc", "filesystems", fs_f);
    free(fs_f);
}

uint32_t vfs_read(vfs_node *node, uint32_t offset, uint32_t size, char *buffer)
{
    if (node && node->read && buffer != NULL)
    {
        uint32_t ret = node->read(node, offset, size, buffer);
        return ret;
    }
    return -1;
}

vfs_fsinfo_t* get_fs_by_uid(uint32_t uid)
{
    if(validate(filesystems) != 1)
        return 0;

    foreach(k, filesystems)
    {
        vfs_fsinfo_t* f = k->val;
        if(f->uid == uid)
            return f;
    }

    return NULL;
}

vfs_fsinfo_t* get_fs_by_name(char* name)
{
    if(validate(filesystems) != 1)
        return 0;

    foreach(k, filesystems)
    {
        vfs_fsinfo_t* f = k->val;
        if(strcmp(f->name, name) == 0)
            return f;
    }

    return NULL;
}

uint32_t find_fs(char* device)
{
    if(validate(filesystems) != 1)
        return 0;

    foreach(k, filesystems)
    {
        vfs_fsinfo_t* fs = k->val;
        if(fs->test && fs->test(device))
        {
            ldprintf("vfs->find_fs", LOG_INFO, "Found %s filesystem on %s", fs->name, device);
            return fs->uid;
        }
    }

    ldprintf("vfs->find_fs", LOG_ERR, "No filesystem found on %s", device);
    return 0;
}

uint32_t vfs_write(vfs_node *file, uint32_t offset, uint32_t size, char *buffer)
{
    if (file && file->write)
    {
        uint32_t ret = file->write(file, offset, size, buffer);
        return ret;
    }
    ldprintf("vfs->write", LOG_ERR, "Cannot write to given file as it not exist or the file is not write enabled");
    return -1;
}

void vfs_open(struct vfs_node *node, uint32_t flags)
{
    if (!node)
    {
        ldprintf("vfs->open", LOG_ERR, "Given file does not exist");
        return;
    }
    if (node->ref_count >= 0)
        node->ref_count++;
    node->open(node, flags);
}

void vfs_close(vfs_node *node)
{
    if (!node || node == vfs_root || node->ref_count == -1)
    {
        //ldprintf("vfs->close", LOG_ERR, "Error occurred during closing the given file");
        return;
    }
    node->ref_count--;
    if (node->ref_count == 0)
        node->close(node);
}

void vfs_chmod(vfs_node *node, uint32_t mode)
{
    if (node->chmod)
        return node->chmod(node, mode);
}

DirectoryEntry *vfs_readdir(vfs_node *node, unsigned int index)
{
    if (node && (node->flags & FS_DIRECTORY) && node->readdir)
        return node->readdir(node, index);
    return NULL;
}

vfs_node *vfs_finddir(vfs_node *node, char *name)
{
    if (node && (node->flags & FS_DIRECTORY) && node->finddir)
        return node->finddir(node, name);
    return NULL;
}

void vfs_mkdir(char *name, unsigned short permission)
{
    int i = strlen(name);
    char *dirname = strdup(name);
    char *save_dirname = dirname;
    char *parent_path = PATH_SEPARATOR_STRING;
    ldprintf("vfs->mkdir", LOG_INFO, "Creating a new directory: %s with permission", name, permission);
    while (i >= 0)
    {
        if (dirname[i] == '/')
        {
            if (i != 0)
            {
                dirname[i] = '\0';
                parent_path = dirname;
            }
            dirname = &dirname[i + 1];
            break;
        }
        i--;
    }

    vfs_node *parent_node = file_open(parent_path, 0);
    if (!parent_node)
    {
        ldprintf("vfs->mkdir", LOG_ERR, "Parent directory of given path does not exist");
        kfree(save_dirname);
    }

    if (parent_node->mkdir)
        parent_node->mkdir(parent_node, dirname, permission);
    kfree(save_dirname);

    vfs_close(parent_node);
}

int vfs_create(char *name, unsigned short permission)
{
    int i = strlen(name);
    char *dirname = strdup(name);
    char *save_dirname = dirname;
    char *parent_path = PATH_SEPARATOR_STRING;
    ldprintf("vfs->create", LOG_INFO, "Creating a new file %s with permission %d", name, permission);
    while (i >= 0)
    {
        if (dirname[i] == '/')
        {
            if (i != 0)
            {
                dirname[i] = '\0';
                parent_path = dirname;
            }
            dirname = &dirname[i + 1];
            break;
        }
        i--;
    }

    vfs_node *parent_node = file_open(parent_path, 0);
    if (!parent_node)
    {
        ldprintf("vfs->create", LOG_ERR, "Parent directory of given path does not exist");
        kfree(save_dirname);
        return -1;
    }
    if (parent_node->create)
        parent_node->create(parent_node, dirname, permission);
    kfree(save_dirname);
    vfs_close(parent_node);
    return 0;
}

void vfs_unlink(char *name)
{
    int i = strlen(name);
    char *dirname = strdup(name);
    char *save_dirname = dirname;
    char *parent_path = PATH_SEPARATOR_STRING;
    while (i >= 0)
    {
        if (dirname[i] == '/')
        {
            if (i != 0)
            {
                dirname[i] = '\0';
                parent_path = dirname;
            }
            dirname = &dirname[i + 1];
            break;
        }
        i--;
    }

    vfs_node *parent_node = file_open(parent_path, 0);
    if (!parent_node)
    {
        kfree(save_dirname);
    }
    if (parent_node->unlink)
        parent_node->unlink(parent_node, dirname);
    kfree(save_dirname);
    vfs_close(parent_node);
}

char *expand_path(char *input)
{
    list_t *input_list = str_split(input, PATH_SEPARATOR_STRING, NULL);
    char *ret = list2str(input_list, PATH_SEPARATOR_STRING);
    return ret;
}

vfs_node *get_mountpoint_recur(char **path, treenode_t *subroot)
{
    int found = 0;
    char *curr_token = strsep(path, PATH_SEPARATOR_STRING);
    if (curr_token == NULL || !strcmp(curr_token, ""))
    {
        struct vfs_entry *ent = (struct vfs_entry *)subroot->data;
        return ent->file;
    }
    foreach (child, subroot->children)
    {
        treenode_t *tchild = (treenode_t *)child->val;
        struct vfs_entry *ent = (struct vfs_entry *)(tchild->data);
        if (strcmp(ent->name, curr_token) == 0)
        {
            found = 1;
            subroot = tchild;
            break;
        }
    }

    if (!found)
    {
        *path = curr_token;
        return ((struct vfs_entry *)(subroot->data))->file;
    }
    return get_mountpoint_recur(path, subroot);
}

vfs_node *get_mountpoint(char **path)
{
    if (strlen(*path) > 1 && (*path)[strlen(*path) - 1] == '/')
        *(path)[strlen(*path) - 1] = '\0';
    if (!*path || *(path)[0] != '/')
        return NULL;
    if (strlen(*path) == 1)
    {
        *path = "\0";
        vfs_entry *ent = (vfs_entry *)vfs_tree->root->data;
        return ent->file;
    }
    (*path)++;
    return get_mountpoint_recur(path, vfs_tree->root);
}


vfs_node *file_open(char *file_name, uint32_t flags)
{
    // ldprintf("VFS", LOG_DEBUG, "Opening file: %s", file_name);
    char *curr_token = NULL;
    char *filename = strdup(file_name);
    char *free_filename = filename;
    char *save = strdup(filename);
    char *original_filename = filename;
    char *new_start = NULL;
    vfs_node *nextnode = NULL;
    vfs_node *startpoint = get_mountpoint(&filename);
    if(strcmp(file_name, "/") == 0)
    {
        kfree(free_filename);
        return vfs_root;
    }

    if (!startpoint)
        return NULL;
    if (filename)
        new_start = strstr(save + (filename - original_filename), filename);
    while (filename != NULL && ((curr_token = strsep(&new_start, PATH_SEPARATOR_STRING)) != NULL))
    {
        nextnode = vfs_finddir(startpoint, curr_token);
        startpoint = nextnode;
        if(strcmp(nextnode->name, "/") == 0)
        {
            continue;
        }
    }
    if (!nextnode)
        nextnode = startpoint;
    if (!nextnode)
    {
        if(((flags & OPEN_WRONLY) == OPEN_WRONLY) || ((flags & OPEN_RDWR) == OPEN_RDWR))
        {
            vfs_create(save, ((current_process == NULL) ? 777 : current_process->umask));
            return file_open(save, flags);
        }
    }
    vfs_open(nextnode, flags);
    kfree(save);
    kfree(free_filename);
    return nextnode;
}

void init_vfs()
{
    ldprintf("vfs", LOG_INFO, "Initializing...");
    vfs_tree = tree_create();
    filesystems = list_create();
    struct vfs_entry *root = (vfs_entry*)kmalloc(sizeof(struct vfs_entry));
    root->name = strdup("root");
    root->file = NULL;
    tree_insert(vfs_tree, NULL, root);
    ldprintf("vfs", LOG_INFO,  "Virtual FileSystem successfully initialized");
}

void vfs_mountDev(char *mountpoint, vfs_node *node)
{
    vfs_mount(mountpoint, node);
}

void vfs_mount_recur(char *path, treenode_t *subroot, vfs_node *fs_obj)
{
    int found = 0;
    char *curr_token = strsep(&path, PATH_SEPARATOR_STRING);

    if (curr_token == NULL || !strcmp(curr_token, ""))
    {
        struct vfs_entry *ent = (struct vfs_entry *)subroot->data;
        if (ent->file)
        {
            ldprintf("vfs->mount", LOG_ERR, "A Device is already mounted at the path. Please unmount and try again.");
            return;
        }
        if (!strcmp(ent->name, PATH_SEPARATOR_STRING)) vfs_root = fs_obj;
        ent->file = fs_obj;
        return;
    }

    foreach (child, subroot->children)
    {
        treenode_t *tchild = (treenode_t *)child->val;
        struct vfs_entry *ent = (struct vfs_entry *)(tchild->data);
        if (strcmp(ent->name, curr_token) == 0)
        {
            found = 1;
            subroot = tchild;
        }
    }

    if (!found)
    {
        vfs_entry *ent = (vfs_entry*)kcalloc(1, sizeof(vfs_entry));
        ent->name = strdup(curr_token);
        subroot = tree_insert(vfs_tree, subroot, ent);
    }
    vfs_mount_recur(path, subroot, fs_obj);
}

void vfs_mount(char *path, vfs_node *fs_obj)
{
    fs_obj->ref_count = -1;
    fs_obj->fs_type = 0;
    ldprintf("vfs->mount", LOG_INFO, "Mounted given node to %s", path);
    if (path[0] == '/' && strlen(path) == 1)
    {
        vfs_entry *ent = (vfs_entry *)vfs_tree->root->data;
        if (ent->file)
        {
            ldprintf("vfs->mount", LOG_ERR, "A Device is already mounted at the path. Please unmount and try again.");
            return;
        }
        vfs_root = fs_obj;
        ent->file = fs_obj;
        return;
    }
    vfs_mount_recur(path + 1, vfs_tree->root, fs_obj);
}

void vfs_unmount_recur(char *path, treenode_t *subroot)
{
    int found = 0;
    char *curr_token = strsep(&path, PATH_SEPARATOR_STRING);
    FILE* fs_obj = NULL;

    if (curr_token == NULL || !strcmp(curr_token, ""))
    {
        struct vfs_entry *ent = (struct vfs_entry *)subroot->data;
        if (!strcmp(ent->name, PATH_SEPARATOR_STRING)) vfs_root = fs_obj;
        ent->file = fs_obj;
        return;
    }

    foreach (child, subroot->children)
    {
        treenode_t *tchild = (treenode_t *)child->val;
        struct vfs_entry *ent = (struct vfs_entry *)(tchild->data);
        if (strcmp(ent->name, curr_token) == 0)
        {
            found = 1;
            subroot = tchild;
        }
    }

    if (!found)
    {
        vfs_entry *ent = (vfs_entry*)kcalloc(1, sizeof(vfs_entry));
        ent->name = strdup(curr_token);
        subroot = tree_insert(vfs_tree, subroot, ent);
    }
    vfs_unmount_recur(path, subroot);
}

void vfs_unmount(char *path)
{
    if (path[0] == '/' && strlen(path) == 1)
    {
        vfs_entry *ent = (vfs_entry *)vfs_tree->root->data;
        ent->file = NULL;
        return;
    }
    vfs_unmount_recur(path + 1, vfs_tree->root);
}

int vfs_ioctl(FILE* f, int req, void* data)
{
    ldprintf("vfs->ioctl", LOG_DEBUG, "Got ioctl request %d", req);
    if(validate(data) != 1)
        return -1;
    if(f && f->ioctl)
        return f->ioctl(f, req, data);
}