#include "system.h"
#include "printf.h"
#include "modules.h"
#include "serial.h"

static int init(int argc, char** argv)
{
    ldprintf("Module Test", LOG_WARN, "Hello kernel");
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