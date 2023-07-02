#include "system.h"
#include "printf.h"
#include "modules.h"
#include "serial.h"
#include "syscall.h"

static int test_syscall_handler(char* str)
{
    ldprintf("Test syscall handler", LOG_ERR, "Got message: %s", str);
    return 0;
}

static int init(int argc, char** argv)
{
    ldprintf("Module Test", LOG_WARN, "Hello kernel");
    register_syscall(test_syscall_handler, MAX_KERNEL_SYSCALLS+1);

    SYSCALL2(MAX_KERNEL_SYSCALLS+1, "Hello Syscall manager", 0, 0);
    return 0;
}

static void test_function()
{
    ldprintf("Test function module", LOG_INFO, "Calling this function from other module");
}

static int fini()
{
    return 0;
}

MODULE_DEF(test, init, fini);