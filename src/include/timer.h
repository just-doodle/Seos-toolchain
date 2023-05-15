#ifndef __TIMER_HAL_H__
#define __TIMER_HAL_H__

#include "system.h"
#include "kheap.h"
#include "string.h"
#include "ports.h"
#include "printf.h"

#define TIMER_INTERFACE_PIT 0xFACD

typedef uint32_t (*timer_get_ticks_t)();
typedef void (*timer_set_frequency_t)(uint32_t hz);

typedef struct timer_interface_struct
{
    uint16_t type;
    uint32_t frequency;
    timer_get_ticks_t get_ticks;
    timer_set_frequency_t set_frequency;
    listnode_t* self;
}timer_interface_t;

typedef void (*wakeup_callback)(void);

typedef struct
{
    wakeup_callback callback;
    double seconds;
    uint32_t ticks;
}wakeup_info_t;

void init_timer_interface();
void register_timer_interface(timer_interface_t* interface);

uint32_t get_ticks();
void set_frequency(uint32_t hz);

void sleep(uint32_t ms);
void register_wakeup_callback(wakeup_callback callback, double seconds);

void timer_interface_call();

#endif /*__TIMER_HAL_H__*/