#include "vboxguest.h"
#include "ifb.h"
#include "draw.h"

static vbox_t vbox;
static uint32_t* vmmdev;

#define VMM_Event_DisplayChange (1 << 2)

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
    unsigned int x = ((unsigned int)vbox.mouse->x * vesa_getXResolution()) / 0xFFFF;
    unsigned int y = ((unsigned int)vbox.mouse->y * vesa_getYResolution()) / 0xFFFF;

    canvas_t c = canvas_create(vesa_getXResolution(), vesa_getYResolution(), ifb_getIFB());
    set_pixel(&c, rgb(170, 170, 170), x, y);
}

void vbox_handle_irq(registers_t* regs)
{
    if(!(vmmdev[2]))
    {
        ldprintf("VBOX", LOG_INFO, "Got irq from others %d", regs->ino);
        return 0;
    }

    vbox.ev->events = vmmdev[2];
    outl(vbox.port, vbox.ev_paddr);
    ldprintf("VBOX", LOG_INFO, "Got irq %d", regs->ino);

    if(vbox.ev->events & VMM_Event_DisplayChange)
        vbox_do_modset();
    if(vbox.ev->events & VMM_Event_Mouse)
        vbox_do_mouse();
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

void init_vbox()
{
    if(pci_isDeviceAvailable(VBOX_VENDOR_ID, VBOX_DEVICE_ID))
    {
        vbox.vbox_pci = pci_get_device(VBOX_VENDOR_ID, VBOX_DEVICE_ID, -1);
        vbox.port = pci_read(vbox.vbox_pci, PCI_OFF_BAR0) & 0xFFFFFFFC;
        vbox.irq_line = pci_read(vbox.vbox_pci, PCI_OFF_INTERRUPT_LINE);
        vmmdev = (pci_read(vbox.vbox_pci, PCI_OFF_BAR1));
        alloc_region(kernel_page_dir, (((uint32_t)vmmdev) & 0xFFFFFFF0), ((((uint32_t)vmmdev) & 0xFFFFFFF0) + (4 * 256 * PAGE_SIZE)), 1, 1, 1);
        ldprintf("VBOX", LOG_INFO, "vmmdev: 0x%x", (uint32_t)vmmdev);

        register_interrupt_handler(IRQ(vbox.irq_line), vbox_handle_irq);

        uint32_t ginfo_paddr;
        vbox_guest_info_t* ginfo = kmalloc_p(sizeof(vbox_guest_info_t), &ginfo_paddr);
        ginfo->header.req_type = VBOX_REQUEST_GUEST_INFO;
        ginfo->header.size = sizeof(vbox_guest_info_t);
        ginfo->header.version = VBOX_REQUEST_HEADER_VERSION;
        ginfo->header.ret_code = 0;
        ginfo->header.reserved[0] = ginfo->header.reserved[1] = 0;
        ginfo->version = VBOX_VMMDEV_VERSION;
        ginfo->os_type = 0;
        outl(vbox.port, ginfo_paddr);

        uint32_t gcaps_paddr;
        vbox_guest_caps_t* gcaps = kmalloc_p(sizeof(vbox_guest_caps_t), &gcaps_paddr);
        gcaps->header.req_type = VBOX_REQUEST_SET_GUEST_CAPS;
        gcaps->header.size = sizeof(vbox_guest_caps_t);
        gcaps->header.version = VBOX_REQUEST_HEADER_VERSION;
        gcaps->header.ret_code = 0;
        gcaps->header.reserved[0] = gcaps->header.reserved[1] = 0;
        gcaps->caps = 1 << 2;
        outl(vbox.port, gcaps_paddr);

        vbox.ev_paddr = 0;
        vbox.ev = kmalloc_p(sizeof(vbox_ack_events_t), &vbox.ev_paddr);
        vbox.ev->header.req_type = VBOX_REQUEST_ACK_EVENTS;
        vbox.ev->header.size = sizeof(vbox_ack_events_t);
        vbox.ev->header.version = VBOX_REQUEST_HEADER_VERSION;
        vbox.ev->header.ret_code = 0;
        vbox.ev->header.reserved[0] = vbox.ev->header.reserved[1] = 0;
        vbox.ev->events = 0;

        vbox.vdc_paddr = 0;
        vbox.vdc = kmalloc_p(sizeof(vbox_display_change_t), &vbox.vdc_paddr);
        vbox.vdc->header.req_type = VBOX_REQUEST_GET_DISPLAY_CHANGE;
        vbox.vdc->header.size = sizeof(vbox_display_change_t);
        vbox.vdc->header.version = VBOX_REQUEST_HEADER_VERSION;
        vbox.vdc->header.ret_code = 0;
        vbox.vdc->header.reserved[0] = vbox.vdc->header.reserved[1] = 0;
        vbox.vdc->height = 0;
        vbox.vdc->width = 0;
        vbox.vdc->bpp = 0;
        vbox.vdc->eventack = 1;
        
        vbox.mouse_paddr = 0;
        vbox.mouse = kmalloc_p(sizeof(vbox_mouse_t), &vbox.mouse_paddr);
        vbox.mouse->header.req_type = VBOX_REQUEST_SET_MOUSE;
        vbox.mouse->header.size = sizeof(vbox_mouse_t);
        vbox.mouse->header.version = VBOX_REQUEST_HEADER_VERSION;
        vbox.mouse->header.ret_code = 0;
        vbox.mouse->header.reserved[0] = vbox.mouse->header.reserved[1] = 0;
        vbox.mouse->features = (1 << 0) | (1 << 4);
        vbox.mouse->x = vbox.mouse->y = 0;

        outl(vbox.port, vbox.mouse_paddr);
        vbox.mouse->header.req_type = VBOX_REQUEST_GET_MOUSE;

        vbox_write("Hello VBOX!", 12);

        vmmdev[3] = (0xFFFFFFFF);
    }
}