#include <fcntl.h>
#include <user_syscall.h>

typedef struct pargs_struct
{
    char** argv;
    int argc;
}pargs_t;
 
extern void exit(int code);
extern int main(int argc, char** argv);
 
void _start()
{
    pargs_t* args;
    SYSCALL_GET_ARGS(args);
    _init_signal();
    int ex = main(args->argc, args->argv);
    exit(ex);
}