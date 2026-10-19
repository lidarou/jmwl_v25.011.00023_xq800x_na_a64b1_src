/*
 * Copyright (c) 2021 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x10fb42
#define LOG_MODULE    AMOD_AOS

#include <time.h>
#include <rtc.h>
#include <aic_errno.h>

#define SECS_PER_MIN         60
#define MINS_PER_HOUR        60
#define SECS_PER_HOUR        3600  /* 60 * 60 */
#define SECS_PER_DAY         86400 /* 60 * 60 * 24 */
#define SECS_PER_NORMAL_YEAR 31536000 /* 60 * 60 * 24 * 365 */
#define DAYS_PER_WEEK        7
#define DAYS_PER_NORMAL_YEAR 365
#define DAYS_PER_LEAP_YEAR   366
#define BEGIN_WEEKDAY        4
#define TIME_ZONE_MAX        720 /* UTC-12:00 , the last time zone */
#define TIME_ZONE_MIN        (-840) /* UTC+14:00 , the first time zone */

#define LEAPS_THRU_END_OF(y) ((y)/4 - (y)/100 + (y)/400)

s32 aic_get_current_time_zone(void);

/**
 * Tells whether a year is leap year or not
 * @param year a year
 * @return 0: not leap year; 1: leap year
 */
static bool is_leap_year(uint32_t year)
{
    return (!(year % 4) && (year % 100)) || !(year % 400);
}

time_t time(time_t *tloc)
{
    struct rtc_time rtctime = {};
    u32 days;
    time_t tv_sec;

    gx_rtc_get_time(&rtctime);
    days = rtc_tm_to_days(&rtctime);
    tv_sec = days * SECS_PER_DAY + rtctime.tm_hour * SECS_PER_HOUR + rtctime.tm_min * MINS_PER_HOUR + rtctime.tm_sec;

    if (tloc) {
        *tloc = tv_sec;
    }

    return tv_sec;
}

time_t mktime(struct tm *tmptr)
{
    time_t timeInSeconds;
    u32 days, year, newdays;
    s32 tz;

    static const unsigned short rtc_ydays[2][13] = {
        /* Normal years */
        { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
        /* Leap years */
        { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
    };

    if (tmptr == NULL) {
        return AIC_EPERM;
    }

    /* tm_isdst is not supported and is ignored */
    if (tmptr->tm_year < (EPOCH_YEAR - TM_YEAR_BASE) ||
        tmptr->tm_sec > 60 || tmptr->tm_sec < 0 ||      /* Seconds [0-60] */
        tmptr->tm_min > 59 || tmptr->tm_min < 0 ||      /* Minutes [0-59] */
        tmptr->tm_hour > 23 || tmptr->tm_hour < 0 ||    /* Hours [0-23] */
        tmptr->tm_mday > 31 || tmptr->tm_mday < 1 ||    /* Day of the month [1-31] */
        tmptr->tm_mon > 11 || tmptr->tm_mon < 0) {      /* Month [0-11] */
        return AIC_EPERM;
    }

    year = tmptr->tm_year + TM_YEAR_BASE - EPOCH_YEAR;
    days = year * 365
            + LEAPS_THRU_END_OF(tmptr->tm_year + TM_YEAR_BASE - 1)
            - LEAPS_THRU_END_OF(EPOCH_YEAR - 1);

    newdays = rtc_ydays[is_leap_year(tmptr->tm_year + TM_YEAR_BASE)][tmptr->tm_mon] + tmptr->tm_mday - 1;

    days = newdays + days;

    timeInSeconds = days * SECS_PER_DAY + tmptr->tm_hour * SECS_PER_HOUR + tmptr->tm_min * SECS_PER_MIN + tmptr->tm_sec;

    tz = aic_get_current_time_zone();

#ifdef __TM_GMTOFF
    tmptr->__TM_GMTOFF = tz;
#endif
#ifdef __TM_ZONE
    tmptr->__TM_ZONE = NULL;
#endif

    timeInSeconds -= tz * SECS_PER_HOUR;

    return timeInSeconds;
}

int gettimeofday(struct timeval *__restrict tv, void *__restrict _tz)
{
    struct rtc_time rtctime = {};
    u32 days;
    struct timezone *tz = (struct timezone *)_tz;

    gx_rtc_get_time(&rtctime);
    days = rtc_tm_to_days(&rtctime);

    if(tv)
    {
        tv->tv_sec = days * SECS_PER_DAY + rtctime.tm_hour * SECS_PER_HOUR + rtctime.tm_min * SECS_PER_MIN + rtctime.tm_sec;
        tv->tv_usec = rtctime.tm_msec * 1000 + rtctime.tm_usec;
    }

    if(tz)
    {
        tz->tz_dsttime = 0;
        tz->tz_minuteswest = 0;
    }

    return AIC_SUCCESS;
}
