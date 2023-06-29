# Modules

SectorOS-RW4 now supports loading kernel modules. Kernel modules are used to implement support to devices which kernel does not support.

A module requires functions which which will be called at the start of module or when unloading the module. These functions can be defined along with the name of the module by the `MODULE_DEF` macro in `<include/modules.h>`.

```c
#include <include/modules.h>

static int init(int argc, char** argv)
{
    /*Add the code which will be executed at the start of the module*/
    return 0;
}

static int fini()
{
    /*Add the code which will be executed when the module unloads*/
    return 0;
}

MODULE_DEF(example, init, fini);
```

## Dependencies

If the module depends on another module, you can list the dependencies by using the `MODULE_DEPENDS` macro in `<include/modules.h>`.

```c
MODULE_DEF(extension, init, fini);
MODULE_DEPENDS(extension, example);
```

The module dependencies are tested on loading time. The kernel does not load the dependencies.

## Kernel functions

The modules can call all non-static functions in the kernel. For example, a module may use the logging functions:

```C
#include <include/modules.h>
#include <include/logdisk.h>

static int init(int argc, char** argv)
{
    ldprintf("Sample module", LOG_WARN, "Hello World!");
    return 0;
}

static int fini()
{
    return 0;
}

MODULE_DEF(log_mod, init, fini);
```

## Problems

* The macro `MODULE_DEPENDS` will need the name of the module using the macro.
