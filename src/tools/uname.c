#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>

int main(int argc, char** argv)
{
    struct utsname* un = malloc(sizeof(struct utsname));
    uname(un);
    if(argc < 2 || (strcmp(argv[1], "-s") == 0) || (strcmp(argv[1], "--kernel-name") == 0))
    {
        printf("%s\n", un->sysname);
        return 0;
    }

    if((strcmp(argv[1], "-a") == 0) || (strcmp(argv[1], "--all") == 0))
    {
        printf("%s %s %s %s %s %s\n", un->sysname, un->nodename, un->release, un->version, un->machine, un->domainname);
        return 0;
    }

    if((strcmp(argv[1], "-n") == 0) || (strcmp(argv[1], "--nodename") == 0))
    {
        printf("%s\n", un->nodename);
        return 0;
    }

    if((strcmp(argv[1], "-v") == 0) || (strcmp(argv[1], "--kernel-version") == 0))
    {
        printf("%s\n", un->version);
        return 0;
    }

    if((strcmp(argv[1], "-r") == 0) || (strcmp(argv[1], "--kernel-release") == 0))
    {
        printf("%s\n", un->release);
        return 0;
    }

    if((strcmp(argv[1], "-m") == 0) || (strcmp(argv[1], "--machine") == 0))
    {
        printf("%s\n", un->machine);
        return 0;
    }

    printf("%s: extra operand '%s'\n", argv[0], argv[1]);
    return 1;
}