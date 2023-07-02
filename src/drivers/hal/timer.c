#include "timer.h"
#include "rtc.h"
#include "process.h"

list_t *timer_interfaces;
list_t* wakeup_list;

timer_interface_t* current_timer_interface = NULL;

int isSleeping = 0;

long internal_ticks_seconds = 0;
long internal_ticks_mseconds = 0;
long internal_ticks;
uint32_t internal_counter01;
uint32_t internal_counter02;

long timezone_sec = 0;

uint64_t year;
uint32_t month;

long timeofday;

int enable_timer_interface = 0;

#include "kernelfs.h"

void init_timer_interface()
{
    timezone_sec = 19800;
    timer_interfaces = list_create();
    wakeup_list = list_create();
    enable_timer_interface = 1;
    ticks = 0;
    internal_counter01 = current_timer_interface->frequency;
    ASM_FUNC("divl %%ebx" : "=a"(internal_counter02) : "a"(current_timer_interface->frequency), "b"(1000), "d"(0));
    timeofday = gettimeofday_internal() + timezone_sec;
    rtc_read(NULL, NULL, NULL, NULL, &month, &year);
    kernelfs_add_variable("/proc", "ticksinceboot", &internal_ticks_mseconds, sizeof(long));
}

void timer_interface_call()
{
    if(!(list_size(timer_interfaces) == 0) && (current_timer_interface != NULL) && (enable_timer_interface == 1))
    {
        if(wakeup_list != NULL)
        {
            for(register listnode_t* l = wakeup_list->head; l != NULL; l = l->next)
            {
                wakeup_info_t* w = l->val;
                if(internal_ticks >= w->ticks)
                {
                    w->ticks = internal_ticks + current_timer_interface->frequency * w->seconds;
                    w->callback();
                }
            }
        }

        if(internal_counter02 == 0)
        {
            ASM_FUNC("divl %%ebx" : "=a"(internal_counter02) : "a"(current_timer_interface->frequency), "b"(1000), "d"(0));
            ASM_FUNC("incl %0" : "+r"(internal_ticks_mseconds));
            if((current_process) && !isSleeping)
                ASM_FUNC("incl %0" : "+r"(current_process->ticks_since_boot));
        }

        if(internal_counter01 == 0)
        {
            internal_counter01 = current_timer_interface->frequency;
            ASM_FUNC("incl %0" : "+r"(internal_ticks_seconds));
            ASM_FUNC("incl %0" : "+r"(timeofday));
        }
        ASM_FUNC("decl %0" : "+r"(internal_counter01));
        ASM_FUNC("decl %0" : "+r"(internal_counter02));
        ASM_FUNC("incl %0" : "+r"(internal_ticks));
    }
}

void sleep(uint32_t ms)
{
    uint32_t ticks = internal_ticks_mseconds + ms;
    ASM_FUNC("sti");
    isSleeping = 1;
    while(internal_ticks_mseconds < ticks)
    {
        ASM_FUNC("pause");
    }
    isSleeping = 0;
}

uint32_t get_frequency()
{
    if(current_timer_interface != NULL)
        return current_timer_interface->frequency;
}

void register_timer_interface(timer_interface_t* interface)
{
    interface->self = list_insert_front(timer_interfaces, interface);
    current_timer_interface = interface;
    internal_counter01 = current_timer_interface->frequency;
    internal_ticks = 0;
    ASM_FUNC("divl %%ebx" : "=a"(internal_counter02) : "a"(current_timer_interface->frequency), "b"(1000), "d"(0));
}

void register_wakeup_callback(wakeup_callback callback, double seconds)
{
    wakeup_info_t* w = ZALLOC_TYPES(wakeup_info_t);
    w->callback = callback;
    w->seconds = seconds;
    w->ticks = internal_ticks + seconds * current_timer_interface->frequency;
    list_push(wakeup_list, w);
}

uint32_t get_ticks()
{
    if((list_size(timer_interfaces) != 0) && (current_timer_interface != NULL) && (current_timer_interface->get_ticks != NULL))
    {
        uint32_t ret = current_timer_interface->get_ticks();
        return ret;
    }
}

void set_frequency(uint32_t hz)
{
    if((list_size(timer_interfaces) != 0) && (current_timer_interface != NULL) && (current_timer_interface->set_frequency != NULL))
    {
        current_timer_interface->set_frequency(hz);
        current_timer_interface->frequency = hz;
    }
}

void printtime()
{
    uint32_t time_s = timeofday;
    uint32_t n = time_s;

    uint32_t days = (n) / (24 * 3600);
    n = n % (24 * 3600);

    uint32_t hours = (n) / 3600;
    n = n % 3600;

    uint32_t minutes = (n) / 60;
    n = n % 60;

    uint32_t seconds = n;

    printf("%02d:%02d:%02d | DAY: %d\n", hours, minutes, seconds, days);
}

int gettimeofday(struct timeval * tp, void *tzp)
{
    tp->tv_sec = timeofday;
    tp->tv_usec = 0; // Not implemented
    return 0;
}

int isPM = 0;

void getDateTimeFromTimestamp(long timestamp, int* day, int* month, int* year, int* hour, int* minutes)
{
    const int secondsPerMinute = 60;
    const int secondsPerHour = 60 * secondsPerMinute;
    const int secondsPerDay = 24 * secondsPerHour;
    int daysPerYear = 365;
    const int secondsPerYear = daysPerYear * secondsPerDay;

    *year = 1970 + timestamp / secondsPerYear;

    int remainingSeconds = timestamp % secondsPerYear;

    int isLeapYear = (*year % 4 == 0 && *year % 100 != 0) || *year % 400 == 0;
    if (isLeapYear)
        daysPerYear++;

    *day = remainingSeconds / secondsPerDay;
    remainingSeconds %= secondsPerDay;

    *hour = remainingSeconds / secondsPerHour;
    remainingSeconds %= secondsPerHour;

    *minutes = remainingSeconds / secondsPerMinute;

    int daysInMonth[] = {31, isLeapYear ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int i;
    for (i = 0; i <= 12; i++) {
        if (*day < daysInMonth[i])
            break;
        *day -= daysInMonth[i];
    }

    *month = i + 1;
}

void timetochar(char* str, int format)
{
    int day = 0;
    int month = 0;
    int year = 0;
    int hours = 0;
    int minutes = 0;

    getDateTimeFromTimestamp(timeofday, &day, &month, &year, &hours, &minutes);

    if(format == 0)
    {
        // DD/MM/YY HH:mm:ss
        memset(str, 0, strlen(str));
        sprintf(str, "%02d/%d/%04d %02d:%02d", day, month, year, hours, minutes);
    }
}

long gettimeofday_seconds()
{
    return timeofday;
}