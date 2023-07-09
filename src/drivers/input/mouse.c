#include "mouse.h"

mouse_info_t mouse_info;

void mouse_write(uint8_t cmd)
{
    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, 0xD4);
    mouse_wait(1);
    outb(MOUSE_DATA_PORT, cmd);
}

uint8_t mouse_read()
{
    mouse_wait(0);
    return inb(MOUSE_DATA_PORT);
}

void mouse_wait(uint8_t type)
{
    uint32_t timeout = 1000000;

    if(type == 0)
    {
        while(timeout--)
        {
            if((inb(MOUSE_COMMAND_PORT) & 1) == 1)
            {
                return;
            }
        }
        return;
    }
    else
    {
        while(timeout--)
        {
            if((inb(MOUSE_COMMAND_PORT) & 2) == 0)
            {
                return;
            }
        }
        return;
    }
}

int left_button_down()
{
    return !mouse_info.p_buttons[0] && mouse_info.c_buttons[0];
}

int right_button_down()
{
    return !mouse_info.p_buttons[2] && mouse_info.c_buttons[2];
}

int left_button_up()
{
    return mouse_info.p_buttons[0] && !mouse_info.c_buttons[0];
}

int right_button_up()
{
    return mouse_info.p_buttons[2] && !mouse_info.c_buttons[2];
}

void mouse_handle_interrupt(registers_t* regs)
{
    static uint8_t mouse_cycle = 0;
    static char mouse_byte[3];

    switch(mouse_cycle)
    {
    case 0:
    {
        mouse_byte[0] = mouse_read();
        if(MOUSE_LEFT_BUTTON(mouse_byte[0]))
        {
            mouse_info.c_buttons[0] = 1;
        }
        else
        {
            mouse_info.c_buttons[0] = 0;
        }

        if(MOUSE_RIGHT_BUTTON(mouse_byte[0]))
        {
            mouse_info.c_buttons[1] = 1;
        }
        else
        {
            mouse_info.c_buttons[1] = 0;
        }

        if(MOUSE_MIDDLE_BUTTON(mouse_byte[0]))
        {
            mouse_info.c_buttons[2] = 1;
        }
        else
        {
            mouse_info.c_buttons[2] = 0;
        }

        mouse_cycle++;
    }break;
    case 1:
    {
        mouse_byte[1] = mouse_read();
        mouse_cycle++;
    }break;
    case 2:
    {
        mouse_byte[2] = mouse_read();
        mouse_info.x = mouse_info.x + (mouse_byte[1]);
        mouse_info.y = mouse_info.y - (mouse_byte[2]);

        if(mouse_info.x < 0)
            mouse_info.x = 0;
        if(mouse_info.y < 0)
            mouse_info.y = 0;

        if(mouse_info.x > (mouse_info.scr_width-1))
            mouse_info.x = (mouse_info.scr_width-1);
        if(mouse_info.y > (mouse_info.scr_height-1))
            mouse_info.y = (mouse_info.scr_height-1);

        set_pixel(&mouse_info.canvas, rgb(255, 255, 255), mouse_info.x, mouse_info.y);
        set_pixel(&mouse_info.canvas, rgb(255, 255, 255), mouse_info.x+1, mouse_info.y);
        set_pixel(&mouse_info.canvas, rgb(255, 255, 255), mouse_info.x, mouse_info.y+1);
        set_pixel(&mouse_info.canvas, rgb(255, 255, 255), mouse_info.x+1, mouse_info.y+1);

        ldprintf("Mouse", LOG_DEBUG, "Mouse is at (%d, %d)", mouse_info.x, mouse_info.y);

        mouse_cycle = 0;
    }break;
    };

    if(mouse_cycle == 0)
    {
        if(mouse_info.onMove)
            mouse_info.onMove(mouse_info.x, mouse_info.y, left_button_up(), right_button_down(), 0);

        memcpy(mouse_info.p_buttons, mouse_info.c_buttons, 3);
        memset(mouse_info.c_buttons, 0x00, 3);
    }
}

void register_mouse_handler(mouse_onMove handler)
{
    mouse_info.onMove = handler;
}

void init_mouse()
{
    mouse_info.scr_width = video_get_width();
    mouse_info.scr_height = video_get_height();

    mouse_info.x = 20;
    mouse_info.y = 20;

    mouse_info.canvas = canvas_create(mouse_info.scr_width, mouse_info.scr_height, ifb_getIFB());

    uint8_t status = 0;

    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, 0xA8);

    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, 0x20);

    mouse_wait(0);
    status=(inb(MOUSE_DATA_PORT) | 2);
    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, 0x60);
    mouse_wait(1);
    outb(MOUSE_DATA_PORT, status);

    mouse_write(0xF6);
    mouse_read();

    mouse_write(0xF4);
    mouse_read();

    register_interrupt_handler(IRQ(12), mouse_handle_interrupt);
}