#ifndef __BIT_H__
#define __BIT_H__

#include "system.h"

uint8_t lobyte(uint16_t data);
uint8_t hibyte(uint16_t data);

void set_lobyte(uint16_t* data, uint8_t num);
void set_hibyte(uint16_t* data, uint8_t num);

int get_bit(uint32_t data, int num);

#endif /*__BIT_H__*/