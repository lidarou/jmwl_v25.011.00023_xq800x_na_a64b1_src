/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the AIC OSA time function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x16d3c5
#define LOG_MODULE    AMOD_AOS

#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif
#include <aic_osa.h>

#ifndef WIN32
#include <timer_inc.h>
#endif

#if defined (CONFIG_PMIC_RTC)
#include <rtc.h>
s32 aic_get_time_zone_in_quarters(void);
void aic_adjust_time_with_time_zone(aos_systime *current_time, s32 time_zone);
#endif

#ifndef WIN32
#if (TX_TIMER_TICKS_PER_SECOND != 1000)
#error  TX_TIMER_TICKS_PER_SECOND must define it to 1000 for aic osa.
#endif
#else
u32 aos_get_time_tick(void)
{
    return (u32)tx_time_get();
}

/**
* aos_time_to_tick() - Translate time(ms) to tick.
* @time: Time value(ms) to translate.
* Return: Tick value.
*/
u32 aos_time_to_tick(u32 time)
{
    u32 mininum;

    if (time == TX_NO_WAIT || time == TX_WAIT_FOREVER)
         return time;

    mininum = 1000 / TX_TIMER_TICKS_PER_SECOND;
#if defined(WIN32) && (defined(CONFIG_COMM_PS) || defined(CONFIG_COMM_PHY))
    /* modify win32 timer tick for PS_IT/PHY_IT to reduce test time */
    mininum = mininum * 2;
#endif
    if (time < mininum)
        time = mininum;

    return time / mininum;
}

/**
* aos_tick_to_time() - Translate tick to time(ms).
* @tick: Tick to translate.
* Return: Time value(ms) .
*/
u32 aos_tick_to_time(u32 tick)
{
    u32 mininum;

    mininum = 1000 / TX_TIMER_TICKS_PER_SECOND;

#if defined(WIN32) && (defined(CONFIG_COMM_PS) || defined(CONFIG_COMM_PHY))
    /* modify win32 timer tick for PS_IT/PHY_IT to reduce test time */
    mininum = mininum * 2;
#endif

    if (!mininum)
        mininum = 1;

    return tick * mininum;
}
#endif /* WIN32 */

/**
* aos_get_utctime() - Get utc time for AP SYS.
* @time: to store the utc time.
*
* Return: void.
*/
void aos_get_utctime(aos_systime *time)
{
#ifdef CONFIG_PMIC_RTC
    AOS_ASSERT(time);
    gx_rtc_get_time((struct rtc_time *)time);
#else
    time->year = 1970;
    time->month = 1;
    time->day = 1;
    time->wday = 1;
    time->hour = 0;
    time->minute = 0;
    time->second = 0;
    time->msecond = 0;
    time->usecond = 0;
#endif
}

/**
* aos_set_utctime() - Set utc time for AP SYS.
* @time: utc time to set.
*
* Return: void.
*/
void aos_set_utctime(aos_systime *time)
{
#if defined (CONFIG_PMIC_RTC) && (CONFIG_SUBSYS_AP)
    AOS_ASSERT(time);
    gx_rtc_set_time((struct rtc_time *)time);
#endif
}

/**
* aos_get_systime() - Get local time for AP SYS.
* @time: to store the local time.
*
* Return: void.
*/
void aos_get_systime(aos_systime *time)
{
#if defined (CONFIG_PMIC_RTC) && (CONFIG_SUBSYS_AP)
    s32 tz;

    AOS_ASSERT(time);

    /* get utc time */
    gx_rtc_get_time((struct rtc_time *)time);
    /* get time zone */
    tz = aic_get_time_zone_in_quarters();
    /* calculate local time */
    aic_adjust_time_with_time_zone(time, tz);
#else
    time->year = 1970;
    time->month = 1;
    time->day = 1;
    time->wday = 1;
    time->hour = 0;
    time->minute = 0;
    time->second = 0;
    time->msecond = 0;
    time->usecond = 0;
#endif
}

/**
* aos_set_systime() - Set local time for AP SYS.
* @time: local time to set.
*
* Return: void.
*/
void aos_set_systime(aos_systime *time)
{
#if defined (CONFIG_PMIC_RTC) && (CONFIG_SUBSYS_AP)
    s32 tz;

    AOS_ASSERT(time);

    /* get time zone */
    tz = aic_get_time_zone_in_quarters();
    /* calculate utc time */
    aic_adjust_time_with_time_zone(time, 0 - tz);
    /* set utc time */
    gx_rtc_set_time((struct rtc_time *)time);
#endif
}

#if defined (CONFIG_PMIC_RTC) && (CONFIG_SUBSYS_AP)
/**
* aos_time_change_request() - Request time change callback.
* If aos time is changed, it will call the callback funtion.
* @cb: callback function.
* @param: callback paramater
*
* Return: void.
*/
void aos_time_change_request(void *cb, void *param)
{
    gx_rtc_time_change_request(cb, param);
}

/**
* aos_time_change_unrequest() - Unrequest time change callback.
*
* Return: void.
*/
void aos_time_change_unrequest(void)
{
    gx_rtc_time_change_unrequest();
}

/**
* aos_get_utc_alarm_time() - Get utc alarm time for AP SYS.
* @time: to store the utc time.
*
* Return: void.
*/
void aos_get_utc_alarm_time(aos_systime *time)
{
    AOS_ASSERT(time);
    gx_rtc_get_alarm((struct rtc_time *)time);
}

/**
* aos_set_utc_alarm_time() - Set utc alarm time for AP SYS.
* @time: utc time to set.
*
* Return: void.
*/
void aos_set_utc_alarm_time(aos_systime *time, void *cb, void *param)
{
    AOS_ASSERT(time);
    gx_rtc_set_alarm((struct rtc_time *)time, (RTC_ALARM_CB)cb, param);
}

/**
* aos_get_alarm_time() - Get local alarm time for AP SYS.
* @time: to store the local time.
*
* Return: void.
*/
void aos_get_alarm_time(aos_systime *time)
{
    s32 tz;

    AOS_ASSERT(time);

    /* get utc alarm time */
    gx_rtc_get_alarm((struct rtc_time *)time);
    /* get time zone */
    tz = aic_get_time_zone_in_quarters();
    /* calculate local time */
    aic_adjust_time_with_time_zone(time, tz);
}

/**
* aos_set_alarm_time() - Set local alarm time for AP SYS.
* @time: local time to set.
*
* Return: void.
*/
void aos_set_alarm_time(aos_systime *time, void *cb, void *param)
{
    s32 tz;

    AOS_ASSERT(time);

    /* get time zone */
    tz = aic_get_time_zone_in_quarters();
    /* calculate utc time */
    aic_adjust_time_with_time_zone(time, 0 - tz);
    /* set utc time */
    gx_rtc_set_alarm((struct rtc_time *)time, (RTC_ALARM_CB)cb, param);
}

/**
* aos_del_alarm_time() - Delete local alarm time.
*
* Return: void.
*/
void aos_del_alarm_time(void)
{
    gx_rtc_del_alarm();
}
#endif

boolean aos_is_leap_year(u32 year)
{
    return (year % 4) || ((year % 100 == 0) && (year % 400)) ? 0 : 1;
}

/**
* aos_adjust_time_with_time_zone - Adjust time with time zone.
* @current_time: current time.
* @time_zone: time zone
*
* Ajust the time with time zone and leap year.
* The time zone is of the range -12~12.
*/
void aos_adjust_time_with_time_zone(aos_systime *current_time, s32 time_zone)
{
    u32 lastday = 0;
    s32 added_hour = 0;
    boolean leap_year;

    if (!current_time)
        return;

    leap_year = aos_is_leap_year(current_time->year);

    if(time_zone > 0) {
        if(current_time->month == 1 || current_time->month == 3 ||
           current_time->month == 5 || current_time->month == 7 ||
           current_time->month == 8 || current_time->month == 10 ||
           current_time->month == 12) {
            lastday = 31;
        } else if (current_time->month == 4 || current_time->month == 6 ||
                   current_time->month == 9 || current_time->month == 11) {
            lastday = 30;
        } else {
            if(leap_year)
                lastday = 29;
            else
                lastday = 28;
        }

        if(current_time->hour + time_zone >= 24) {
            current_time->hour = current_time->hour + time_zone - 24;
            current_time->day += 1;
            if(current_time->day > lastday) {
                current_time->day -= lastday;
                current_time->month += 1;
                if(current_time->month > 12) {
                    current_time->month -= 12;
                    current_time->year += 1;
                }
            }
            current_time->wday += 1;
            if (current_time->wday > 6)
                current_time->wday = 0;
        } else {
            current_time->hour += time_zone;
        }
    } else if (time_zone < 0) {
        added_hour = current_time->hour + time_zone;

        if(added_hour < 0) {
            current_time->hour = added_hour + 24;
            current_time->day -= 1;
            if(current_time->day == 0) {
                current_time->month -= 1;
                if(current_time->month == 1 || current_time->month == 3 ||
                   current_time->month == 5 || current_time->month == 7 ||
                   current_time->month == 8 || current_time->month == 10 ||
                   current_time->month == 0) {
                    lastday = 31;
                } else if (current_time->month == 4 ||
                           current_time->month == 6 ||
                           current_time->month == 9 ||
                           current_time->month == 11) {
                    lastday = 30;
                } else {
                    if(leap_year)
                        lastday = 29;
                    else
                        lastday = 28;
                }
                current_time->day += lastday;
                if(current_time->month == 0) {
                    current_time->day = 31;
                    current_time->month += 12;
                    current_time->year -= 1;
                }
            }

            if (current_time->wday >= 1)
                current_time->wday -= 1;
            else
                current_time->wday = 6;
        } else {
            current_time->hour += time_zone;
        }
    }
}

