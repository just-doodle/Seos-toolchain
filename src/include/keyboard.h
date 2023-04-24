#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "system.h"
#include "isr.h"
#include "ports.h"
#include "string.h"
#include "printf.h"

#define KEYBOARD_COMMAND_PORT 0x64
#define KEYBOARD_DATA_PORT 0x60

typedef void (*keyboard_handler_t)(uint8_t scancode);

void init_keyboard();
void keyboard_callback(registers_t *regs);
void change_keyboard_handler(keyboard_handler_t handler);
void change_keyboard_handler2(keyboard_handler_t handler);
char kcodeTochar(uint8_t scancode);
void use_handler(int state);

#endif /*__KEYBOARD_H__*/