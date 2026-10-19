/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of rtc core, it is rtc lib
 *    common driver.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1d7b7b
#define LOG_MODULE    AMOD_RTC

#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_type.h>

#include <rtc.h>

#define RTC_BASE_YEAR       1970

static const unsigned char rtc_days_in_month[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static const unsigned short rtc_ydays[2][13] = {
    /* Normal years */
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    /* Leap years */
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

#define LEAPS_THRU_END_OF(y) ((y)/4 - (y)/100 + (y)/400)

/**
 * Tells whether a year is leap year or not
 * @param year a year
 * @return 0: not leap year; 1: leap year
 */
static bool is_leap_year(uint32_t year)
{
    return (!(year % 4) && (year % 100)) || !(year % 400);
}

/*
 * The number of days in the month.
 */
int rtc_month_days(unsigned int month, unsigned int year)
{
    return rtc_days_in_month[month] + (is_leap_year(year) && month == 1);
}

/*
 * The number of days since January 1. (0 to 365)
 */
int rtc_year_days(unsigned int day, unsigned int month, unsigned int year)
{
    return rtc_ydays[is_leap_year(year)][month] + day-1;
}

/*
 * rtc_day_to_tm - Converts day to rtc_time.
 * Convert days since 01-01-1970 to Gregorian date.
 */
void rtc_day_to_tm(u32 days, struct rtc_time *tm)
{
    int days_temp = (int)days;
    unsigned int month, year, ldays;

    /* day of the week, 1970-01-01 was a Thursday */
    tm->tm_wday = (days + 4) % 7;

    year = RTC_BASE_YEAR + days / 365;
    ldays = LEAPS_THRU_END_OF(year - 1) - LEAPS_THRU_END_OF(RTC_BASE_YEAR - 1);
    if (!(days % 365) && ldays) {
        year -= 1;
        //days += is_leap_year(year);
    }
    days_temp -= (year - RTC_BASE_YEAR) * 365
        + LEAPS_THRU_END_OF(year - 1)
        - LEAPS_THRU_END_OF(RTC_BASE_YEAR - 1);
    if (days_temp < 0) {
        year -= 1;
        days_temp += 365 + is_leap_year(year);
    }
    tm->tm_year = year;

    for (month = 0; month < 11; month++) {
        int newdays;

        newdays = days_temp - rtc_month_days(month, year);
        if (newdays < 0)
            break;
        days_temp = newdays;
    }
    tm->tm_mon = month + 1;
    tm->tm_day = days_temp + 1;
}

/*
 * check rtc param is valid or not.
 */
int rtc_valid_tm(struct rtc_time *tm)
{
    if (tm->tm_year < RTC_BASE_YEAR
        || ((unsigned)tm->tm_mon) > 12
        || tm->tm_day > rtc_month_days(tm->tm_mon - 1, tm->tm_year)
        || ((unsigned)tm->tm_hour) >= 24
        || ((unsigned)tm->tm_min) >= 60
        || ((unsigned)tm->tm_sec) >= 60)
        return AIC_EPERM;

    return 0;
}

/*
 * rtc_tm_to_days - Converts rtc_time to days.
 * Convert Gregorian date to seconds since 01-01-1970.
 */
u32 rtc_tm_to_days(struct rtc_time *tm)
{
    u32 days, year, newdays;

    if (rtc_valid_tm(tm))
        return AIC_EPERM;

    year = tm->tm_year - RTC_BASE_YEAR;

    days = year * 365
        + LEAPS_THRU_END_OF(tm->tm_year - 1)
        - LEAPS_THRU_END_OF(RTC_BASE_YEAR - 1);


    newdays = rtc_year_days(tm->tm_day, tm->tm_mon - 1, tm->tm_year);

    days = newdays + days;

    return days;
}

/*
 * rtc_tm_to_time64 - Converts rtc_time to sec.
 * Convert Gregorian date to seconds since 01-01-1970.
 */
u64 rtc_tm_to_time64(struct rtc_time *tm)
{
    u32 days;
    u64 sec;

    days = rtc_tm_to_days(tm);

    sec = (u64)((u64)days * 24U * 3600U
          + (u64)tm->tm_hour * 3600U
          + (u64)tm->tm_min * 60U
          + (u64)tm->tm_sec);

    return sec;
}

/*
 * rtc_time64_to_tm - Converts sec to rtc_time.
 * Convert seconds to day from Gregorian 01-01-1970.
 */
void rtc_time64_to_tm(u64 sec, struct rtc_time *tm)
{
    u32 days, sec_tmp;

    AIC_ASSERT(tm);

    days = (u32)(sec/(24 * 3600));
    sec_tmp = (u32)(sec%(24 * 3600));
    tm->tm_hour = sec_tmp/3600;
    sec_tmp = sec_tmp%3600;
    tm->tm_min = sec_tmp/60;
    sec_tmp = sec_tmp%60;
    tm->tm_sec = sec_tmp;

    rtc_day_to_tm(days, tm);
}

/*
 * rtc_tm_to_ms - Converts rtc_time to milliseconds.
 * Convert Gregorian date to milliseconds since 01-01-1970.
 */
u64 rtc_tm_to_ms(struct rtc_time *tm)
{
    u64 sec;

    AIC_ASSERT(tm);

    sec = rtc_tm_to_time64(tm);

    return sec * 1000;
}

/*
 * rtc_ms_to_tm - Converts milliseconds to rtc_time.
 * Convert milliseconds to day from Gregorian 01-01-1970.
 */
void rtc_ms_to_tm(u64 msec, struct rtc_time *tm)
{
    u64 sec, msec_tmp;

    AIC_ASSERT(tm);

    msec_tmp = msec%1000;
    sec = msec/1000;
    tm->tm_msec = (u32)msec_tmp;

    rtc_time64_to_tm(sec, tm);
}

