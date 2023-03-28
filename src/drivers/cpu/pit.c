#include "pit.h"

uint32_t ticks;
uint16_t pit_frequency;
uint32_t countdown;
uint16_t pit_oldfreq;

void init_pit()
{
    printf("[PIT] Initializing PIT...\n");
    register_interrupt_handler(IRQ(0), pit_callback);
    ticks = 0;
    printf("[PIT] Frequency set to 1hz\n");
    pit_chfrequency(1); // 1 pulse every 1 second
    printf("[PIT] Initialization successful\n");
}

void pit_callback(registers_t* regs)
{
    ticks++;
    countdown--;
}

void pit_chfrequency(uint16_t hz)
{
    pit_frequency = hz;
    uint8_t l = lobyte((PIT_CHANNEL0_FREQUENCY/pit_frequency));
    uint8_t h = hibyte((PIT_CHANNEL0_FREQUENCY/pit_frequency));

    /*
    * 0x36 = 00 11 011 0
    * HEX  = CH AC MOD B
    */
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, l);
    outb(PIT_CHANNEL0, h);
}

void sleep(uint32_t ms)
{
    pit_oldfreq = pit_frequency;
    pit_chfrequency(1000);
    countdown = ms;
    while(countdown > 0)
    {
        asm volatile("hlt");
    }
    pit_chfrequency(pit_oldfreq);
}