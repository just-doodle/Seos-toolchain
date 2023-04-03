#ifndef __RDTSC_H__
#define __RDTSC_H__

#include "system.h"
#include "pit.h"

#define c_la	((PIT_CHANNEL0_FREQUENCY+100/2)/100) 
#define calibrate_tsc	(c_la*5)
#define calibrate_time	(5*1000020/100)


#define readtsc(low,high) __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

ul_t calculate_tsc();

#endif /*__RDTSC_H__*/