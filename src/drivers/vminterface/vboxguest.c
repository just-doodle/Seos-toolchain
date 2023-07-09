#include "vboxguest.h"
#include "ifb.h"
#include "compositor.h"
#include "draw.h"

static vbox_t vbox;
static uint32_t* vmmdev;

#define VMM_Event_DisplayChange (1 << 2)

#define VBOX_USE_MOUSE_INTEGRATION 0

void vbox_do_modset()
{
    outl(vbox.port, vbox.vdc_paddr);
    outl(vbox.port, vbox.vdc_paddr);
    video_modeset(vbox.vdc->width, vbox.vdc->height, vbox.vdc->bpp);
}

#define VMM_Event_Mouse (1 << 9)
void vbox_do_mouse()
{
    outl(vbox.port, vbox.mouse_paddr);
    unsigned int x = ((unsigned int)vbox.mouse->x * video_get_width()) / 0xFFFF;
    unsigned int y = ((unsigned int)vbox.mouse->y * video_get_height()) / 0xFFFF;
}

void vbox_handle_irq(registers_t* regs)
{
    if(!(vmmdev[2]))
    {
        ldprintf("VBOX", LOG_INFO, "Got irq from others %d", regs->ino);
        return;
    }

    vbox.ev->events = vmmdev[2];
    outl(vbox.port, vbox.ev_paddr);

    if(vbox.ev->events & VMM_Event_DisplayChange)
        vbox_do_modset();
#if VBOX_USE_MOUSE_INTEGRATION
    if(vbox.ev->events & VMM_Event_Mouse)
        vbox_do_mouse();
#endif
}

#define EARLY_LOG_DEVICE 0x504
size_t vbox_write(uint8_t * buffer, size_t size)
{
	for (unsigned int i = 0; i < size; ++i)
    {
		outb(EARLY_LOG_DEVICE, buffer[i]);
	}
	return size;
}

void vbox_add_window_seamless(window_t* w)
{
    vbox_rtrect_t* rect = zalloc(sizeof(vbox_rtrect_t));
    rect->xLeft = w->x;
    rect->xRight = w->x+w->width;
    rect->yTop = w->y;
    rect->yBottom = w->y+w->height;

    list_push(vbox.rects, rect);

    vbox.visReg->count++;

    vbox.visReg->rects[vbox.rect_count] = *rect;
    vbox.rect_count++;
    vbox.visReg->header.size = sizeof(vbox_visibleregion_t) + (sizeof(vbox_rtrect_t) * vbox.rect_count);

    outl(vbox.port, vbox.visReg_paddr);
}

void vbox_add_seamless_region(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    vbox_rtrect_t* rect = zalloc(sizeof(vbox_rtrect_t));
    rect->xLeft = x;
    rect->xRight = x+width;
    rect->yTop = y;
    rect->yBottom = y+height;

    list_push(vbox.rects, rect);

    vbox.visReg->count++;

    memcpy(&vbox.visReg->rects[vbox.rect_count], rect, sizeof(vbox_rtrect_t));
    vbox.rect_count++;
    vbox.visReg->header.size = sizeof(vbox_visibleregion_t) + (sizeof(vbox_rtrect_t) * vbox.rect_count);

    outl(vbox.port, vbox.visReg_paddr);
}

void init_vbox()
{
    if(pci_isDeviceAvailable(VBOX_VENDOR_ID, VBOX_DEVICE_ID))
    {
        vbox.vbox_pci = pci_get_device(VBOX_VENDOR_ID, VBOX_DEVICE_ID, -1);
        vbox.port = pci_read(vbox.vbox_pci, PCI_OFF_BAR0) & 0xFFFFFFFC;
        vbox.irq_line = pci_read(vbox.vbox_pci, PCI_OFF_INTERRUPT_LINE);
        vmmdev = (uint32_t*)(pci_read(vbox.vbox_pci, PCI_OFF_BAR1));
        alloc_region(kernel_page_dir, (((uint32_t)vmmdev) & 0xFFFFFFF0), ((((uint32_t)vmmdev) & 0xFFFFFFF0) + (4 * 256 * PAGE_SIZE)), 1, 1, 1);
        ldprintf("VBOX", LOG_INFO, "vmmdev: 0x%x", (uint32_t)vmmdev);

        register_interrupt_handler(IRQ(vbox.irq_line), vbox_handle_irq);

        uint32_t ginfo_paddr;
        vbox_guest_info_t* ginfo = (vbox_guest_info_t*)kmalloc_p(sizeof(vbox_guest_info_t), &ginfo_paddr);
        ginfo->header.req_type = VBOX_REQUEST_GUEST_INFO;
        ginfo->header.size = sizeof(vbox_guest_info_t);
        ginfo->header.version = VBOX_REQUEST_HEADER_VERSION;
        ginfo->header.ret_code = 0;
        ginfo->header.reserved[0] = ginfo->header.reserved[1] = 0;
        ginfo->version = VBOX_VMMDEV_VERSION;
        ginfo->os_type = 0;
        outl(vbox.port, ginfo_paddr);

        uint32_t gcaps_paddr;
        vbox_guest_caps_t* gcaps = (vbox_guest_caps_t*)kmalloc_p(sizeof(vbox_guest_caps_t), &gcaps_paddr);
        gcaps->header.req_type = VBOX_REQUEST_SET_GUEST_CAPS;
        gcaps->header.size = sizeof(vbox_guest_caps_t);
        gcaps->header.version = VBOX_REQUEST_HEADER_VERSION;
        gcaps->header.ret_code = 0;
        gcaps->header.reserved[0] = gcaps->header.reserved[1] = 0;
        gcaps->caps = 1 << 2 | (1 << 0);
        outl(vbox.port, gcaps_paddr);

        vbox.ev_paddr = 0;
        vbox.ev = (vbox_ack_events_t*)kmalloc_p(sizeof(vbox_ack_events_t), &vbox.ev_paddr);
        vbox.ev->header.req_type = VBOX_REQUEST_ACK_EVENTS;
        vbox.ev->header.size = sizeof(vbox_ack_events_t);
        vbox.ev->header.version = VBOX_REQUEST_HEADER_VERSION;
        vbox.ev->header.ret_code = 0;
        vbox.ev->header.reserved[0] = vbox.ev->header.reserved[1] = 0;
        vbox.ev->events = 0;

        vbox.vdc_paddr = 0;
        vbox.vdc = (vbox_display_change_t*)kmalloc_p(sizeof(vbox_display_change_t), &vbox.vdc_paddr);
        vbox.vdc->header.req_type = VBOX_REQUEST_GET_DISPLAY_CHANGE;
        vbox.vdc->header.size = sizeof(vbox_display_change_t);
        vbox.vdc->header.version = VBOX_REQUEST_HEADER_VERSION;
        vbox.vdc->header.ret_code = 0;
        vbox.vdc->header.reserved[0] = vbox.vdc->header.reserved[1] = 0;
        vbox.vdc->height = 1280;
        vbox.vdc->width = 768;
        vbox.vdc->bpp = 32;
        vbox.vdc->eventack = 1;
        
#if VBOX_USE_MOUSE_INTEGRATION
        vbox.mouse_paddr = 0;
        vbox.mouse = (vbox_mouse_t*)kmalloc_p(sizeof(vbox_mouse_t), &vbox.mouse_paddr);
        vbox.mouse->header.req_type = VBOX_REQUEST_SET_MOUSE;
        vbox.mouse->header.size = sizeof(vbox_mouse_t);
        vbox.mouse->header.version = VBOX_REQUEST_HEADER_VERSION;
        vbox.mouse->header.ret_code = 0;
        vbox.mouse->header.reserved[0] = vbox.mouse->header.reserved[1] = 0;
        vbox.mouse->features = (1 << 0) | (1 << 4);
        vbox.mouse->x = vbox.mouse->y = 0;

        outl(vbox.port, vbox.mouse_paddr);
        vbox.mouse->header.req_type = VBOX_REQUEST_GET_MOUSE;
#endif

        vbox.rects = list_create();
        vbox.visReg_paddr = 0;

        vbox.visReg = kmalloc_p(sizeof(vbox_visibleregion_t)+(sizeof(vbox_rtrect_t) * 128), &vbox.visReg_paddr);
        vbox.visReg->header.size = sizeof(vbox_visibleregion_t) + (sizeof(vbox_rtrect_t) * 0);
        vbox.visReg->header.version = VBOX_REQUEST_HEADER_VERSION;
        vbox.visReg->header.req_type = VBOX_REQUEST_SET_VISIBLE_REGION;
        vbox.visReg->header.ret_code = 0;
        vbox.visReg->header.reserved[0] = vbox.visReg->header.reserved[1] = 0;
        vbox.visReg->count = 0;
        outl(vbox.port, vbox.visReg_paddr);

        vbox_write(((uint8_t*)"Hello VBOX!"), 12);

        vmmdev[3] = (0xFFFFFFFF);

        vbox_add_window_seamless(get_focused_window());
        vbox_add_seamless_region(video_get_width(), video_get_height(), 0, 0);
    }
}