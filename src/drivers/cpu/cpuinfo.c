#include "cpuinfo.h"
#include "kheap.h"

ul_t calculate_cpuspeed()
{
    ul_t tsc = calculate_tsc();
    ul_t cpu_khz = 0;
    if(tsc)
    {
        ul_t eax=0;
        ul_t edx=1000;
		__asm__("divl %2":"=a" (cpu_khz), "=d" (edx) :"r" (tsc),"0" (eax), "1" (edx));
		printf("[CPUINFO] Detected cpu speed: %d.%d Mhz\n",cpu_khz / 1000, cpu_khz % 1000);
		eax=0;
		edx= 1000000;
		ASM_FUNC ("divl %2" : "=a"(cpu_khz),"=d"(edx):"r"(tsc),"0"(eax),"1"(edx));
		return cpu_khz/1000000;
    }
    else
        return 0;
}

void printregs(int eax, int ebx, int ecx, int edx)
{
	int j;
	char string[17];
	string[16] = '\0';
	for(j = 0; j < 4; j++) {
		string[j] = eax >> (8 * j);
		string[j + 4] = ebx >> (8 * j);
		string[j + 8] = ecx >> (8 * j);
		string[j + 12] = edx >> (8 * j);
	}
	serialprintf("%s\r\n", string);
}

void getstr(char* string, uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx)
{
    printregs(eax, ebx, ecx, edx);
	int j;
	string[16] = '\0';
	for(j = 0; j < 4; j++)
    {
        if(isprint(eax >> (8 * j)))
		    string[j] = eax >> (8 * j);
        if(isprint(ebx >> (8 * j)))
		    string[j + 4] = ebx >> (8 * j);
        if(isprint(ecx >> (8 * j)))
		    string[j + 8] = ecx >> (8 * j);
        if(isprint(edx >> (8 * j)))
		    string[j + 12] = edx >> (8 * j);
	}
}

void getstr_noeax(char* string, uint32_t ebx, uint32_t ecx, uint32_t edx)
{
	string[0] = ebx & 0xFF;
    string[1] = (ebx >> 8) & 0xFF;
    string[2] = (ebx >> 16) & 0xFF;
    string[3] = (ebx >> 24) & 0xFF;
    string[4] = (ecx) & 0xFF;
    string[5] = (ecx >> 8) & 0xFF;
    string[6] = (ecx >> 16) & 0xFF;
	string[7] = (ecx >> 24) & 0xFF;
    string[8] = (edx) & 0xFF;
    string[9] = (edx >> 8) & 0xFF;
    string[10] = (edx >> 16) & 0xFF;
    string[11] = (edx >> 24) & 0xFF;
    string[12] = '\0';
}

cpuinfo_t* get_cpuinfo()
{
    cpuinfo_t* cpu = kmalloc(sizeof(cpuinfo_t));
    memset(cpu, 0, sizeof(cpuinfo_t));
    cpu->cpuSpeed = calculate_cpuspeed();
    uint32_t eax, ebx, ecx, edx, unused, max_eax;
    CPUID(0, eax, ebx, ecx, edx);
    getstr_noeax(cpu->vendorString, ebx, edx, ecx);

    CPUID(0x80000000, max_eax, unused, unused, unused)
    if(max_eax >= 0x80000004)
    {
		if(max_eax >= 0x80000002) {
			CPUID(0x80000002, eax, ebx, ecx, edx)
            getstr(cpu->infoString, eax, ebx, ecx, edx);
		}
		if(max_eax >= 0x80000003) {
			CPUID(0x80000003, eax, ebx, ecx, edx)
			getstr(cpu->infoString + (strlen(cpu->infoString)), eax, ebx, ecx, edx);
		}
		if(max_eax >= 0x80000004) {
			CPUID(0x80000004, eax, ebx, ecx, edx)
			getstr(cpu->infoString + (strlen(cpu->infoString)), eax, ebx, ecx, edx);
		}
    }

    return cpu;
}

void print_cpu_info()
{
    cpuinfo_t* cpu = get_cpuinfo();
    printf("CPU #0: %s\n\tVendor: %s\n\tClock speed: %d.%dGHz\n", cpu->infoString, cpu->vendorString, cpu->cpuSpeed / 1000, cpu->cpuSpeed % 1000);
}