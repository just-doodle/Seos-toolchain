#ifndef __SHELL_H__
#define __SHELL_H__

#include "system.h"
#include "power.h"
#include "string.h"
#include "printf.h"
#include "keyboard.h"
#include "list.h"
#include "math.h"
#include "cpuinfo.h"

typedef int (*shellcmdf_t)(list_t* args);

typedef struct shellcmd
{
    char cmd[32];
    char help[128];
    shellcmdf_t f;
}shellcmd_t;

void register_command(shellcmd_t cmd);
void shell_callback(uint8_t scancode);
void init_shell();

#endif /*__SHELL_H__*/