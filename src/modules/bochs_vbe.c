#include "system.h"
#include "modules.h"

#include "paging.h"
#include "kheap.h"
#include "logdisk.h"
#include "ifb.h"
#include "pci.h"

#define BOCHS_VIDEO_VENDOR_ID_1 0x80EE
#define BOCHS_VIDEO_VENDOR_ID_2 0x1234

#define BOCHS_VIDEO_DEVICE_ID_1 0xBEEF
#define BOCHS_VIDEO_DEVICE_ID_2 0x1111

typedef struct bochs_vbe_driver_info_struct
{
    uint32_t width;
    uint32_t height;
    uint32_t bpp;

    uint32_t pitch;
    uint32_t size;

    uint32_t* fb;

    pci_t device;

    ifb_video_driver_t* driver;
}bochs_vbe_driver_info_t;

static bochs_vbe_driver_info_t bochs_info;

static int bochs_vbe_draw(uint32_t* fb, uint32_t width, uint32_t height)
{
    if(validate(bochs_info.fb) != 1)
        return 1;

    memcpy(bochs_info.fb, fb, bochs_info.size);
    return 0;
}

static ifb_video_info_t* bochs_vbe_get_modeinfo()
{
    ifb_video_info_t* info = zalloc(sizeof(ifb_video_info_t));

    info->bpp = bochs_info.bpp;
    info->height = bochs_info.height;
    info->width = bochs_info.width;

    info->fb = bochs_info.fb;
    info->pitch = bochs_info.pitch;
    info->pix = IFB_PIXELFORMAT_ARGB;
    info->size = bochs_info.size;
    
    return info;
}

static void* bochs_vbe_get_framebuffer()
{
    uint32_t addr = pci_read(bochs_info.device, PCI_OFF_BAR0);
    alloc_kernel_region(addr, addr+bochs_info.size, 1, 1, 1);

    return addr;
}

static int bochs_vbe_modeset(uint32_t width, uint32_t height, uint32_t bpp)
{
    outw(0x1CE, 0x04);
    outw(0x1CF, 0x00);
    outw(0x1CE, 0x01);
    outw(0x1CF, width & 0xFFFF);
    outw(0x1CE, 0x02);
    outw(0x1CF, height & 0xFFFF);
    outw(0x1CE, 0x03);
    outw(0x1CF, bpp & 0xFFFF);
	outw(0x1CE, 0x04);
	outw(0x1CF, 0x41);
	outw(0x1CE, 0x01);

	uint32_t wwidth = inw(0x1CF);

    bochs_info.width = wwidth;
    bochs_info.height = height;
    bochs_info.bpp = bpp;
    bochs_info.pitch = bochs_info.width * (bochs_info.bpp/sizeof(uint32_t));
    bochs_info.size = bochs_info.pitch * bochs_info.height;
    bochs_info.fb = bochs_vbe_get_framebuffer();
    return 0;
}

static int init(int argc, char** argv)
{
    if(!pci_isDeviceAvailable(BOCHS_VIDEO_VENDOR_ID_2, BOCHS_VIDEO_DEVICE_ID_2))
    {
        if(!pci_isDeviceAvailable(BOCHS_VIDEO_VENDOR_ID_1, BOCHS_VIDEO_DEVICE_ID_1))
        {
            ldprintf("Bochs VBE", LOG_ERR, "Bochs VBE adapter is not present in the pci bus.");
            return 1;
        }
        else
            bochs_info.device = pci_get_device(BOCHS_VIDEO_VENDOR_ID_1, BOCHS_VIDEO_DEVICE_ID_1, -1);
    }
    else
        bochs_info.device = pci_get_device(BOCHS_VIDEO_VENDOR_ID_2, BOCHS_VIDEO_DEVICE_ID_2, -1);

    outw(0x1CE, 0x00);
    uint16_t ver = inw(0x1CF);

    if(ver < 0xB0C0 || ver > 0xB0C6)
    {
        ldprintf("Bochs VBE", LOG_ERR, "Reported version of BGA is 0x%x which is not supported.", ver);
        return 1;
    }

    bochs_info.driver = ZALLOC_TYPES(ifb_video_driver_t);
    bochs_info.driver->draw = bochs_vbe_draw;
    bochs_info.driver->get_modeinfo = bochs_vbe_get_modeinfo;
    bochs_info.driver->modeset = bochs_vbe_modeset;
    strcpy(bochs_info.driver->name, "bochs_vbe");

    bochs_vbe_modeset(1024, 768, 32);

    register_video_driver(bochs_info.driver);
    ifb_change_driver("bochs_vbe");

    return 0;
}

static int fini()
{
    ifb_remove_driver("bochs_vbe");
    return 0;
}

MODULE_DEF(bochs_vbe, init, fini);