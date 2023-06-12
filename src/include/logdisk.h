#ifndef __LOGDISK_H__
#define __LOGDISK_H__

#include "system.h"
#include "debug.h"
#include "kheap.h"
#include "string.h"
#include "printf.h"
#include "charbuffer.h"
#include "vfs.h"
#include "devfs.h"

#define LOG_VERBOSE 0
#define LOG_DEBUG 1
#define LOG_INFO 2
#define LOG_WARN 3
#define LOG_ERR  4
#define LOG_OFF 5

typedef struct logdisk_struct
{
    int type;
    charbuffer_t* buf;
}logdisk_t;

void init_logdisk(uint32_t size, int type);
void logdisk_log(char* issuer, char* msg, int type);
void logdisk_dump(char* file);
void logdisk_mount();
void logdisk_change_policy(int policy);

void ldprintf(char* issuer, int type, char* fmt, ...);

#endif /*__LOGDISK_H__*/