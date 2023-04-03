#ifndef __MATH_H__
#define __MATH_H__

#include "system.h"

#define abs(a) (((a) < 0)?-(a):(a))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define sign(x) ((x < 0) ? -1 :((x > 0) ? 1 : 0))
#define is_power_of_two(x) (((x) & ((x) - 1)) == 0)

void seed(uint16_t seed);
void init_rand();
uint32_t rand();
uint32_t rand_range(uint32_t min, uint32_t max);
uint32_t getBIT(uint32_t b, int num);

uint32_t rdrand();
int check_rdrand();

#endif /*__MATH_H__*/