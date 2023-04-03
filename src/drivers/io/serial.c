#include "serial.h"

serialio_t serial;

#define PORT serial.port

int init_serial(uint16_t port, int skip_test)
{
    serial.port = port;
    serial.type = SERIALIO_COMMUNICATION;
    serial.skip_test = skip_test;
    
    
    outb(serial.port + 1, 0x00);
    outb(serial.port + 3, 0x80);
    outb(serial.port + 0, 0x03);
    outb(serial.port + 1, 0x00);
    outb(serial.port + 3, 0x03);
    outb(serial.port + 2, 0xC7);
    outb(serial.port + 4, 0x0B);

    if(skip_test != 1)
    {
        outb(serial.port + 4, 0x1E);
        outb(serial.port + 0, 0xAE);
        if(inb(serial.port + 0) != 0xAE)
        {
            return 1;
        }
    }

    outb(serial.port + 4, 0x0F);
    return 0;
}

int serial_received()
{
    return inb(serial.port + 5) & 1;
}

char read_serial()
{
    while(serial_received() == 0)

    return (inb(serial.port));
}

int is_transmit_empty()
{
    return inb(serial.port + 5) & 0x20;
}

void write_serial(char val)
{
    while(is_transmit_empty() == 0);
    outb(serial.port, val);
}

void serial_putc(char c)
{
    if(c == '\n')
    {
        write_serial('\r');
        write_serial('\n');
        return;
    }
    write_serial(c);
}

void serial_puts(char* str)
{
    while(*str != 0)
    {
        serial_putc(*str);
        str++;
    }
}