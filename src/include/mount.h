#ifndef __MOUNT_H__
#define __MOUNT_H__

#include "system.h"
#include "printf.h"
#include "string.h"

int mount(char* devpath, char* mountpoint);

#endif /*__MOUNT_H__*/