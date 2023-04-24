#include "pit.h"
#include "process.h"

uint32_t ticks;
uint16_t pit_frequency;
uint32_t countdown;
uint16_t pit_oldfreq;

list_t* wakeup_list = NULL;

void init_pit()
{
    printf("[PIT] Initializing PIT...\n");
    register_interrupt_handler(IRQ(0), pit_callback);
    ticks = 0;
    printf("[PIT] Frequency set to 1000hz\n");
    pit_chfrequency(1000); // 1000 pulse every 1 second
    wakeup_list = list_create();
    printf("[PIT] Initialization successful\n");
}

void pit_callback(registers_t* regs)
{
    ticks++;
    countdown--;
    //memcpy(&saved_context, regs, sizeof(registers_t));
    //serialprintf("TICK\n");
    foreach(t, wakeup_list)
    {
        wakeup_info_t* w = (wakeup_info_t*)t->val;
        w->callback();
    }
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
    //serialprintf("[PIT] Sleeping for %d ms.\n", ms);
    countdown = ms;
    while(countdown > 0)
    {
        asm volatile("hlt");
    }
}

uint32_t get_ticks()
{
    return ticks;
}

void pit_register(wakeup_callback callback, double seconds)
{
    uint32_t tick = ticks + seconds * pit_frequency;

    wakeup_info_t* wakeup = zalloc(sizeof(wakeup_info_t));
    wakeup->callback = callback;
    wakeup->seconds = seconds;
    wakeup->ticks = tick;
    list_insert_front(wakeup_list, wakeup);
}