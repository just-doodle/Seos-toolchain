#include "bit.h"

uint8_t lobyte(uint16_t data)
{
    return (data & 0xFF);
}

uint8_t hibyte(uint16_t data)
{
    return ((data >> 8) & 0xFF);
}

void set_hibyte(uint16_t* data, uint8_t num)
{
    *data = (uint16_t)(num << 8);
}

void set_lobyte(uint16_t* data, uint8_t num)
{
    *data = (uint16_t)num;
}

int get_bit(uint32_t data, int num)
{
    int r = (data >> (num - 1)) & 0x01;
    return r;
}

int ibit(uint8_t byte, int nbit)
{
    //0010 >> 1 0001 & 0x1 = 1
    int r = (byte >> (nbit - 1)) & 0x01;
    return r;
}