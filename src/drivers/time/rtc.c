#include "rtc.h"
#include "printf.h"
#include "math.h"

static int century_register = 0;

uint8_t rtc_read_reg(int reg)
{
    outb(CMOS_COMMAND, reg);
    return inb(CMOS_DATA);
}

uint32_t rtc_get_update_in_progres_flag()
{
    outb(CMOS_COMMAND, 0x0A);
    return inb(CMOS_DATA) & 0x80;
}

void rtc_read(uint8_t* second, uint8_t *minute, uint8_t *hour, uint8_t *day, uint8_t *month, uint32_t *year)
{
    uint8_t century;
    uint8_t last_second;
    uint8_t last_minute;
    uint8_t last_hour;
    uint8_t last_day;
    uint8_t last_month;
    uint8_t last_year;
    uint8_t last_century;
    uint8_t registerB;

    while(rtc_get_update_in_progres_flag());
    (*second) = rtc_read_reg(RTC_REG_SECONDS);
    (*minute) = rtc_read_reg(RTC_REG_MINUTES);
    (*hour) = rtc_read_reg(RTC_REG_HOURS);
    (*day) = rtc_read_reg(RTC_REG_DAYS);
    (*month) = rtc_read_reg(RTC_REG_MONTHS);
    (*year) = rtc_read_reg(RTC_REG_YEARS);
    if(century_register != 0)
    {
        century = rtc_read_reg(century_register);
    }

    do
    {
        last_second = (*second);
        last_minute = (*minute);
        last_hour = (*hour);
        last_day = (*day);
        last_month = (*month);
        last_year = (*year);
        last_century = century;
        
        while(rtc_get_update_in_progres_flag());
        (*second) = rtc_read_reg(RTC_REG_SECONDS);
        (*minute) = rtc_read_reg(RTC_REG_MINUTES);
        (*hour) = rtc_read_reg(RTC_REG_HOURS);
        (*day) = rtc_read_reg(RTC_REG_DAYS);
        (*month) = rtc_read_reg(RTC_REG_MONTHS);
        (*year) = rtc_read_reg(RTC_REG_YEARS);
        if(century_register != 0)
        {
            century = rtc_read_reg(century_register);
        }
    }
    while(last_second != (*second) || last_minute != (*minute) || last_hour != (*hour) || last_day != (*day) || last_month != (*month) || last_year != (*year) || last_century != century);

    registerB = rtc_read_reg(RTC_REG_B);

    if(!(registerB & 0x04))
    {
        (*second) = ((*second) & 0x0F) + (((*second) / 16) * 10);
        (*minute) = ((*minute) & 0x0F) + (((*minute) / 16) * 10);
        (*hour) = ( ((*hour) & 0x0F) + ((((*hour) & 0x70) / 16) * 10) ) | ((*hour) & 0x80);
        (*day) = ((*day) & 0x0F) + (((*day) / 16) * 10);
        (*month) = ((*month) & 0x0F) + (((*month) / 16) * 10);
        (*year) = ((*year) & 0x0F) + (((*year) / 16) * 10);
        if(century_register != 0)
        {
            century = (century & 0x0F) + ((century / 16) * 10);
        }
    }

    if(!(registerB & 0x02) && ((*hour) & 0x80))
    {
        (*hour) = (((*hour) & 0x7F) + 12) % 24;
    }

    if(century_register != 0)
    {
        (*year) += century * 100;
    }
    else
    {
        (*year) += (CURRENT_YEAR / 100) * 100;
        if((*year) < CURRENT_YEAR)
        {
            (*year) += 100;
        }
    }
}

void rtc_read_time(rtc_time_t *time)
{
    rtc_read(&time->second, &time->minute, &time->hour, &time->day, &time->month, &time->year);
}

rtc_time_t r;
void rtc_char(char* buffer, int format)
{
    rtc_read_time(&r);
    if(format == 0)
    {
        // strcpy(buffer, itoa_r(r.day, 10));
        // strcat(buffer, "/");
        // strcat(buffer, itoa_r(r.month, 10));
        // strcat(buffer, "/");
        // strcat(buffer, itoa_r(r.year, 10));
        // strcat(buffer, " ");
        // strcat(buffer, itoa_r(r.hour+5, 10));
        // strcat(buffer, ":");
        // strcat(buffer, itoa_r(r.minute+30, 10));
    }
    else if (format == 1)
    {
        sprintf(buffer, "%d/%d/%d %02d:%02d %s", r.day, r.month, r.year, abs(r.hour - 12), r.minute, (r.hour < 12 ? "AM" : "PM"));
    }
}

uint32_t secs_of_years(int years)
{
	uint32_t days = 0;
	//years += 2000;
	while (years > 1969)
    {
		days += 365;
		if (years % 4 == 0)
        {
			if (years % 100 == 0)
            {
				if (years % 400 == 0)
                {
					days++;
				}
			}else
            {
				days++;
			}
		}
		years--;
	}
	return days * 86400;
}

uint32_t secs_of_month(int months, int year)
{
	//year += 2000;

	uint32_t days = 0;
	switch(months)
    {
		case 11:
			days += 30;
		case 10:
			days += 31;
		case 9:
			days += 30;
		case 8:
			days += 31;
		case 7:
			days += 31;
		case 6:
			days += 30;
		case 5:
			days += 31;
		case 4:
			days += 30;
		case 3:
			days += 31;
		case 2:
			days += 28;
			if ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)))
            {
				days++;
			}
		case 1:
			days += 31;
		default:
			break;
	}
	return days * 86400;
}

uint32_t days_of_month(int year, int months)
{
    uint32_t days = 0;
	switch(months)
    {
		case 11:
			days += 30;
		case 10:
			days += 31;
		case 9:
			days += 30;
		case 8:
			days += 31;
		case 7:
			days += 31;
		case 6:
			days += 30;
		case 5:
			days += 31;
		case 4:
			days += 30;
		case 3:
			days += 31;
		case 2:
			days += 28;
			if ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)))
            {
				days++;
			}
		case 1:
			days += 31;
		default:
			break;
	}
	return days;
}

long gettimeofday_internal()
{
    rtc_time_t rtime;
    rtc_read_time(&rtime);
    long time = secs_of_years(rtime.year - 1) + secs_of_month(rtime.month-1, rtime.year) + (rtime.day - 1)*86400 + (rtime.hour)*3600 + (rtime.minute)*60 + (rtime.second) + 0;
    return time;
}