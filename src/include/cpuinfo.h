#ifndef __CPUINFO_H__
#define __CPUINFO_H__

#include "system.h"
#include "rdtsc.h"
#include "printf.h"
#include "cpuid.h"

#define CPUID(in, a, b, c, d) asm("cpuid": "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(in));

typedef struct cpuinfo_struct
{
    char vendorString[13];
    char infoString[51];
    ul_t cpuSpeed; // in KHz
}cpuinfo_t;

ul_t calculate_cpuspeed();
cpuinfo_t* get_cpuinfo();

void print_cpu_info();

#endif /*__CPUINFO_H__*/