#include "math.h"
#include "pit.h"

static uint16_t cseed[3];

int is_rdrand = 0;


void seed(uint16_t num)
{
    cseed[0] = num * ticks;
    cseed[1] = num * ticks;
    cseed[2] = num * ticks;
}

uint32_t prand()
{
    uint32_t x = cseed[0];
    uint32_t y = cseed[1];
    uint32_t z = cseed[2];
    x = x * 0x5DEECE66D + 0xB;
    y = y * 0x5DEECE66D + 0xB;
    z = z * 0x5DEECE66D + 0xB;
    cseed[0] = x;
    cseed[1] = y;
    cseed[2] = z;
    uint32_t ret = x ^ y ^ z;
    
    return ret;
}

uint32_t rand()
{
    uint32_t rng = 0;
    if(is_rdrand)
    {
        rng = rdrand();
    }
    else
    {
        rng = prand();
    }
    return rng;
}

void init_rand()
{
    is_rdrand = check_rdrand();
    if(is_rdrand == 0)
    {
        printf("[RNG] No rdrand support, using PRNG.\n");
        seed(ticks);
        for(int i = 0; i < ticks; i++)
            rand();
    }
}

uint32_t getBIT(uint32_t b, int num)
{
    return (b >> num) & 1;
}

uint32_t rand_range(uint32_t min, uint32_t max)
{
    return rand() % (max - min + 1) + min;
}