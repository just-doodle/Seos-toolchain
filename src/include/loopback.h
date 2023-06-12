#ifndef __LOOPBACK_H__
#define __LOOPBACK_H__

#include "system.h"
#include "string.h"
#include "net.h"
#include "kheap.h"

typedef struct loopback_struct
{
    net_t *dev;
    
    uint32_t ip_addr;
}loopback_t;

#endif /*__LOOPBACK_H__*/