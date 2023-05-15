#ifndef __PIT_H__
#define __PIT_H__

#include "system.h"
#include "isr.h"
#include "printf.h"
#include "bit.h"

#define PIT_CHANNEL0 0x40
#define PIT_CHANNEL1 0x41
#define PIT_CHANNEL2 0x42

#define PIT_COMMAND 0x43

#define PIT_CHANNEL0_FREQUENCY 1193180

extern uint32_t ticks;
extern uint16_t pit_frequency;

void init_pit();
void pit_callback(registers_t *regs);
void pit_chfrequency(uint16_t hz);
void sleep(uint32_t ms);
uint32_t get_ticks();

#endif /*__PIT_H__*/