/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0bc196
#define LOG_MODULE    AMOD_AOS

#include <time.h>
#include <rtc.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <unistd.h>

#if defined (CONFIG_PMIC_RTC)
s32 aic_get_current_time_zone(void);
#endif

#define CPU_TICKS_PER_SECOND   1000
#define SECONDS_IN_DAY      86400//60*60*24
#define SECONDS_IN_HOUR     3600//60*60
#define SECONDS_IN_MINUTE   60

struct tm * localtime_r(const time_t *timep, struct tm *result)
{
    struct rtc_time rtctime;
    u32 days;
    int hours;
    int minutes;
    int seconds;
    time_t tmp_tm = *timep;
    s32 tz = aic_get_current_time_zone();

    tmp_tm += tz * SECONDS_IN_HOUR;
    days = tmp_tm / SECONDS_IN_DAY;
    seconds = tmp_tm % SECONDS_IN_MINUTE;
    minutes = (tmp_tm % SECONDS_IN_HOUR) / SECONDS_IN_MINUTE;
    hours = (tmp_tm % SECONDS_IN_DAY) / SECONDS_IN_HOUR;

    rtc_day_to_tm(days, &rtctime);

    result->tm_hour = hours;                            //hour
    result->tm_min = minutes;                           //minute
    result->tm_sec = seconds;                           //second
    result->tm_mday = rtctime.tm_day;                   //day
    result->tm_mon = rtctime.tm_mon - 1;                //month
    result->tm_year = rtctime.tm_year - TM_YEAR_BASE;   //year
    //weekend
    result->tm_wday = rtctime.tm_wday;
#ifdef __TM_GMTOFF
    result->__TM_GMTOFF = tz;
#endif
#ifdef __TM_ZONE
    result->__TM_ZONE = NULL;
#endif
    return result;
}

int _gettimeofday (struct timeval *p, void *tz)
{
    return gettimeofday(p, tz);
}

int usleep (useconds_t useconds)
{
    return (int)aos_sleep(useconds / 1000);
}

unsigned sleep (unsigned int seconds)
{
    unsigned temp1, temp2, diff, result;

    temp1 = aos_get_tickcount();
    aos_sleep(seconds * CPU_TICKS_PER_SECOND);
    temp2 = aos_get_tickcount();
    diff = temp2 - temp1;

    if (diff > (seconds * CPU_TICKS_PER_SECOND))
    {
        result = (diff - (seconds * CPU_TICKS_PER_SECOND));
    }
    else
    {
        result = ((seconds * CPU_TICKS_PER_SECOND) - diff);
    }

    return (result/CPU_TICKS_PER_SECOND);
}

