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
    data = (num << 8);
}

void set_lobyte(uint16_t* data, uint8_t num)
{
    data = num;
}

int get_bit(uint32_t data, int num)
{
    return (data >> num) & 1;
}