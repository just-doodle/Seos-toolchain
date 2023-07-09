#ifndef __VBOX_GUEST_H__
#define __VBOX_GUEST_H__

#include "system.h"
#include "string.h"
#include "kheap.h"
#include "paging.h"
#include "logdisk.h"
#include "ports.h"
#include "pci.h"
#include "mmio.h"
#include "isr.h"

#define VBOX_VENDOR_ID 0x80EE
#define VBOX_DEVICE_ID 0xCAFE
#define VBOX_VMMDEV_VERSION 0x00010003
#define VBOX_REQUEST_HEADER_VERSION 0x10001
 
#define VBOX_REQUEST_GUEST_INFO 50
#define VBOX_REQUEST_ACK_EVENTS 41
#define VBOX_REQUEST_GET_DISPLAY_CHANGE 51
#define VBOX_REQUEST_GET_MOUSE 1
#define VBOX_REQUEST_SET_MOUSE 2
#define VBOX_REQUEST_SET_GUEST_CAPS 55
#define VBOX_REQUEST_SET_VISIBLE_REGION 71

typedef struct vbox_header_struct
{
    uint32_t size;
    uint32_t version;
    uint32_t req_type;
    int32_t ret_code;
    uint32_t reserved[2];
}vbox_header_t;

typedef struct vbox_guest_info_struct
{
    vbox_header_t header;
    uint32_t version;
    uint32_t os_type;
}vbox_guest_info_t;

typedef struct vbox_guest_caps_struct
{
    vbox_header_t header;
    uint32_t caps;
}vbox_guest_caps_t;

typedef struct vbox_ack_events_struct
{
    vbox_header_t header;
    uint32_t events;
}vbox_ack_events_t;

typedef struct vbox_display_change_struct
{
    vbox_header_t header;
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t eventack;
}vbox_display_change_t;

typedef struct vbox_mouse_struct
{
    vbox_header_t header;
    uint32_t features;
    int32_t x;
    int32_t y;
}vbox_mouse_t;

typedef struct vbox_rtrect_struct
{
    int32_t xLeft;
    int32_t yTop;

    int32_t xRight;
    int32_t yBottom;
}vbox_rtrect_t;

typedef struct vbox_visibleregion_struct
{
    vbox_header_t header;
    uint32_t count;
    vbox_rtrect_t rects[];
}vbox_visibleregion_t;

typedef struct vbox_struct
{
    uint16_t port;
    uint32_t vmmdev_addr;
    uint32_t irq_line;

    vbox_ack_events_t* ev;
    uint32_t ev_paddr;

    vbox_display_change_t* vdc;
    uint32_t vdc_paddr;

    vbox_mouse_t* mouse;
    uint32_t mouse_paddr;

    list_t* rects;
    uint32_t rect_count;

    vbox_visibleregion_t* visReg;
    uint32_t visReg_paddr;

    pci_t vbox_pci;
}vbox_t;

void init_vbox();

#endif /*__VBOX_GUEST_H__*/