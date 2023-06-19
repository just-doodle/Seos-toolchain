#include "system.h"
#include "printf.h"
#include "modules.h"

int init(int argc, char** argv)
{
    serialprintf("Hello kernel.\nThis is a test module.\n");
    return 0;
}

int fini()
{
    return 0;
}

module_info_t metadata = {
    .name = "test",
    .init = init,
    .fini = fini,
};