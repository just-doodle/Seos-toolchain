#include "pit.h"
#include "process.h"
#include "timer.h"

uint32_t ticks;
uint16_t pit_frequency;
uint32_t countdown;
uint16_t pit_oldfreq;

uint32_t pit_getticks()
{
    return ticks;
}

void init_pit()
{
    printf("[PIT] Initializing PIT...\n");
    timer_interface_t* ti = ZALLOC_TYPES(timer_interface_t);
    ti->get_ticks = pit_getticks;
    ti->set_frequency = pit_chfrequency;
    ti->type = TIMER_INTERFACE_PIT;
    ti->frequency = 1000;
    register_timer_interface(ti);
    register_interrupt_handler(IRQ(0), pit_callback);
    ticks = 0;
    printf("[PIT] Frequency set to 1000hz\n");
    pit_chfrequency(1000); // 1000 pulse every 1 second
    printf("[PIT] Initialization successful\n");
}

void pit_callback(registers_t* regs)
{
    ASM_FUNC("cli");
    //ASM_FUNC("incl %0": "+r"(ticks));
    // memcpy(&saved_context, regs, sizeof(registers_t));
    timer_interface_call();
    ASM_FUNC("sti");
}

void pit_chfrequency(uint32_t hz)
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