#include "bit.h"

uint8_t lobyte(uint16_t data)
{
    return (data & 0xFF);
}

uint8_t hibyte(uint16_t data)
{
    return ((data >> 8) & 0xFF);
}