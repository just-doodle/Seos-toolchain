#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/dirent.h>

int ls(char* path)
{
    if(path == NULL)
        return -1;
    DIR *d;
    struct dirent *dir;
    d = opendir(path);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
        return 0;
    }
    return -1;
}

int main(int argc, char** argv)
{
    printf("\033[33mHello World!\033[m\n");
    FILE* f = fopen("/proc/ticksinceboot", "rb");
    if(f != NULL)
    {
        size_t sz = 0;
        fseek(f, 0, SEEK_END);
        sz = ftell(f);
        fseek(f, 0, SEEK_SET);
        if(sz == sizeof(long))
        {
            long ticks;
            fread(&ticks, sz, 1, f);
            printf("Ticks since boot: %ld\n", ticks);
        }
        fclose(f);
    }

    umask(777);
    mount(NULL, "/tmpfs", "tmpfs", 0, 0);
    FILE* q = fopen("/tmpfs/test", "wb+");
    fwrite("Hello Kernel. It is i CommH!\n", 30, 1, q);
    fclose(q);

    FILE* p = fopen("/proc/tmpfs", "rb");
    fseek(p, 0, SEEK_END);
    uint32_t __p_sz = ftell(p);
    fseek(p, 0, SEEK_SET);

    char* bf = malloc(__p_sz);
    fread(bf, __p_sz, 1, p);

    printf("%s", bf);

    fclose(p);

    return 0;
}