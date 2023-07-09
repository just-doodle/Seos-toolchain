#ifndef __MOUSE_H__
#define __MOUSE_H__

#include "system.h"
#include "ports.h"
#include "kheap.h"
#include "draw.h"
#include "logdisk.h"
#include "ifb.h"

typedef int (*mouse_onMove)(uint32_t dx, uint32_t dy, int left, int right, int middle);

#define MOUSE_LEFT_BUTTON(flag) (flag & 0x1)
#define MOUSE_RIGHT_BUTTON(flag) (flag & 0x2)
#define MOUSE_MIDDLE_BUTTON(flag) (flag & 0x4)

#define MOUSE_COMMAND_PORT 0x64
#define MOUSE_DATA_PORT 0x60

#define CURSOR_WIDTH 32
#define CURSOR_HEIGHT 32

typedef struct mouse_info_struct
{
    uint32_t x;
    uint32_t y;

    uint32_t scr_height;
    uint32_t scr_width;

    uint8_t p_buttons[3];
    uint8_t c_buttons[3];

    canvas_t canvas;

    mouse_onMove onMove;
}mouse_info_t;

void init_mouse();

uint8_t mouse_read();
void mouse_write(uint8_t cmd);
void mouse_wait(uint8_t cmd);

void register_mouse_handler(mouse_onMove handler);

void mouse_handle_interrupt(registers_t* regs);

#endif /*__MOUSE_H__*/