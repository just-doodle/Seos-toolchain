#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "system.h"
#include "ports.h"

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

#define SERIALIO_COMMUNICATION 0

typedef struct serialio
{
    uint32_t type;
    uint16_t port;
    int skip_test;
}serialio_t;

int init_serial(uint16_t port, int skip_test);
void serial_putc(char c);
void serial_puts(char* str);

char read_serial();

#endif /*__SERIAL_H__*/