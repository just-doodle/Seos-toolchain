#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "stdarg.h"
#include "stdbool.h"
#include "multiboot.h"

#define __USE_INBUILT_STDINT__ 1

#if __USE_INBUILT_STDINT__
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long int64_t;

#else
#include "stdint.h"
#endif

typedef uint32_t size_t;
typedef unsigned long ul_t;
typedef uint32_t uintptr_t;


#define khalt ASM_FUNC("cli"); \
            ASM_FUNC("hlt");

#define KERNEL_BASE 0xC0000000
#define KERNEL_END  k_end

#define ASM_FUNC(...) __asm__ __volatile__(__VA_ARGS__)

#define NULL (void*)0
/*
* version system
* REVN.YY.MM.RELN(STATUS)
* REVN: revision number (increases when major release)
* YY: first digit of year of release (eg. if year is 2023 then YY=23)
* MM: month of release
* RELN: index of current release in the current month
* STATUS: [PR]:Prerelease, [AL]:alpha, [NR]:Normal release
*/
#define KERNEL_VERSION "7.23.07.1NR"
#define KERNEL_VERSION_CODENAME "Perfect Pomegranate"

#define KERNEL_ENABLED_OPTIONS "\b"

#if __ENABLE_DEBUG_SYMBOL_LOADING__
#undef KERNEL_ENABLED_OPTIONS
#define KERNEL_ENABLED_OPTIONS "ELF_SYMS_LOAD"
#endif

#define KERNEL_BUILD_DATE __DATE__
#define KERNEL_BUILD_TIME __TIME__

#define KERNEL_NAME "SectorOS-RW4"
#define KERNEL_ARCH "x86"

#ifndef KERNEL_COMPILER
#define KERNEL_COMPILER "unknown"
#endif

#define KB 1024
#define MB (1024*KB)
#define GB (1024*MB)

extern uint32_t k_end;

extern multiboot_info_t* m_info;
extern int paging_enabled;
extern bool kheap_enabled;

void reboot();

typedef struct registers
{
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t ino, ecode;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

typedef struct register16
{
    uint16_t di;
    uint16_t si;
    uint16_t bp;
    uint16_t sp;
    uint16_t bx;
    uint16_t dx;
    uint16_t cx;
    uint16_t ax;

    uint16_t ds;
    uint16_t es;
    uint16_t fs;
    uint16_t gs;
    uint16_t ss;
    uint16_t eflags;
}register16_t;

#endif