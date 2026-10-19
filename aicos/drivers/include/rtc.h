/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of rtc module.
 *   It includes all the rtc common function api.
 */
#ifndef __RTC_H__
#define __RTC_H__

#include <aic_bitmap.h>

typedef void (*RTC_ALARM_CB)(void *param);
typedef void (*RTC_CB)(void *param);

typedef enum {
    RTC_INT_NONE = 0,
    RTC_SEC_INT = BIT_0,
    RTC_MIN_INT = BIT_1,
    RTC_HOUR_INT = BIT_2,
    RTC_DAY_INT = BIT_3,
    RTC_UPDATE_DONE = BIT_4,
    RTC_ALARM1_CFG_DONE = BIT_5,
    RTC_ALARM2_CFG_DONE = BIT_6,
    RTC_ALARM1_INT = BIT_7,
    RTC_ALARM2_INT = BIT_8,
} RTC_INT_MODE;

struct rtc_time {
    u32 tm_year;
    u32 tm_mon;
    u32 tm_wday;
    u32 tm_day;
    u32 tm_hour;
    u32 tm_min;
    u32 tm_sec;
    u32 tm_msec;
    u32 tm_usec;
};

#ifdef CONFIG_DRV_RTC_PMIC
/*
 * The number of days in the month.
 */
int rtc_month_days(unsigned int month, unsigned int year);

/*
 * The number of days since January 1. (0 to 365)
 */
int rtc_year_days(unsigned int day, unsigned int month, unsigned int year);

/*
 * rtc_day_to_tm - Converts day to rtc_time.
 * Convert days since 01-01-1970 to Gregorian date.
 */
void rtc_day_to_tm(u32 days, struct rtc_time *tm);
/*
 * check rtc param is valid or not.
 */
int rtc_valid_tm(struct rtc_time *tm);

/*
 * rtc_tm_to_days - Converts rtc_time to days.
 * Convert Gregorian date to seconds since 01-01-1970.
 */
u32 rtc_tm_to_days(struct rtc_time *tm);

/*
 * rtc_tm_to_time64 - Converts rtc_time to sec.
 * Convert Gregorian date to seconds since 01-01-1970.
 */
u64 rtc_tm_to_time64(struct rtc_time *tm);

/*
 * rtc_time64_to_tm - Converts sec to rtc_time.
 * Convert seconds to day from Gregorian 01-01-1970.
 */
void rtc_time64_to_tm(u64 sec, struct rtc_time *tm);

/*
 * rtc_tm_to_ms - Converts rtc_time to milliseconds.
 * Convert Gregorian date to milliseconds since 01-01-1970.
 */
u64 rtc_tm_to_ms(struct rtc_time *tm);

/*
 * rtc_ms_to_tm - Converts milliseconds to rtc_time.
 * Convert milliseconds to day from Gregorian 01-01-1970.
 */
void rtc_ms_to_tm(u64 msec, struct rtc_time *tm);

/*
 * gx_rtc_get_time
 * Get rtc time.
 */
void gx_rtc_get_time(struct rtc_time *tm);
/*
 * gx_rtc_set_time
 * Get rtc time.
 */
void gx_rtc_set_time(struct rtc_time *tm);
/*
 * gx_rtc_time_change_request
 * Request rtc time change callback.
 */
void gx_rtc_time_change_request(void *cb, void *param);
/*
 * gx_rtc_time_change_unrequest
 * Unrequest rtc time change callback.
 */
void gx_rtc_time_change_unrequest(void);
/*
 * gx_rtc_get_alarm
 * Get rtc alarm.
 */
void gx_rtc_get_alarm(struct rtc_time *rm);
/*
 * gx_rtc_set_alarm
 * Set rtc alarm.
 */
void gx_rtc_set_alarm(struct rtc_time *ra, RTC_ALARM_CB cb, void *param);
/*
 * gx_rtc_alarm_enable
 * Enable or disable rtc alarm.
 */
void gx_rtc_alarm_enable(BOOL enable);

/*
 * gx_rtc_del_alarm
 * Delete current rtc alarm.
 */
void gx_rtc_del_alarm(void);
#ifdef CONFIG_FP
signed char gx_rtc_alarm_param(void);//获取当前是哪个闹钟在等待中，包括闹钟和备忘录
#endif

#else
static inline int rtc_month_days(unsigned int month, unsigned int year) {return 0;}
static inline int rtc_year_days(unsigned int day, unsigned int month, unsigned int year) {return 0;}
static inline void rtc_day_to_tm(u32 days, struct rtc_time *tm) {}
static inline int rtc_valid_tm(struct rtc_time *tm) {return 0;}
static inline u32 rtc_tm_to_days(struct rtc_time *tm) {return 0;}
static inline u64 rtc_tm_to_time64(struct rtc_time *tm) {return 0;}
static inline void rtc_time64_to_tm(u64 sec, struct rtc_time *tm) {}
static inline u64 rtc_tm_to_ms(struct rtc_time *tm) {return 0;}
static inline void rtc_ms_to_tm(u64 msec, struct rtc_time *tm) {}
static inline void gx_rtc_get_time(struct rtc_time *tm) {}
static inline void gx_rtc_set_time(struct rtc_time *tm) {}
static inline void gx_rtc_time_change_request(void *cb, void *param) {}
static inline void gx_rtc_time_change_unrequest(void) {}
static inline void gx_rtc_get_alarm(struct rtc_time *rm) {}
static inline void gx_rtc_set_alarm(struct rtc_time *ra, RTC_ALARM_CB cb, void *param) {}
static inline void gx_rtc_alarm_enable(BOOL enable) {}
static inline void gx_rtc_del_alarm(void) {}
#endif

#endif

