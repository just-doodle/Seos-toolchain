#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "stdint.h"
#include "stdarg.h"
#include "stdbool.h"

extern uint32_t k_end;

void panic(char* message, char* file);
void kernel_panic(char* message);

#define khalt asm("cli") asm("hlt")

#define KERNEL_START 0x100000
#define KERNEL_END  k_end
/*
* version system
* REVN.YY.MM.RELN(STATUS)
* REVN: revision number (increases when major release)
* YY: first digit of year of release (eg. if year is 2023 then YY=23)
* MM: month of release
* RELN: index of current release in the current month
* STATUS: [PR]:Prerelease, [AL]:alpha, [NR]:Normal release
*/
#define KERNEL_VERSION "0.23.03.1ALPR"

void reboot();

#endif