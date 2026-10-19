

/**
 *  DESCRIPTION
 *
 *    2024.6.25  author:
 */
#ifndef __SETTINGS_UI_H__
#define __SETTINGS_UI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"
#include "stdlib.h"
#include "aic_osa.h"

    /* Repeat type */
    typedef enum {
        ALM_REPEAT_OFF,                             /* off */
        ALM_REPEAT_ONCE,                            /* once */
        ALM_REPEAT_HOURLY,                          /* hourly */
        ALM_REPEAT_EVERYDAY,                        /* everyday */
        ALM_REPEAT_DAYS,                            /* days */
        ALM_REPEAT_WEEKLY,                          /* weekly */
        ALM_REPEAT_MONTHLY,                         /* monthly */
        ALM_REPEAT_SEASONLY,                        /* seasonly */
        ALM_REPEAT_YEARLY,                          /* yearly */
        ALM_REPEAT_TITHI,                           /* tithi */
        ALM_REPEAT_CUSTOM,                          /* customized repeat type */
        ALM_REPEAT_TOTAL
        } alm_repeat_enum;


    /* Alarm type */
    typedef enum {
        ALM_TYPE_NONE,
        ALM_TYPE_ALARM,
        ALM_TYPE_SCHEDULE,
        ALM_TYPE_CUSTOM,
        ALM_TYPE_CALENDAR,
        ALM_TYPE_MAX
        } alm_type_enum;


    /* Reminder return value. */
    typedef enum {
        ALM_RET_OK,                                 /* OK */
        ALM_RET_FAIL,                               /* fail */
        ALM_RET_INVALID_DATA,                       /* invalid data */
        ALM_RET_INVALID_TYPE,                       /* type */
        ALM_RET_INVALID_INDEX,                      /* invalid index */
        ALM_RET_INVALID_TIME,                       /* Invalid time or time is smaller than current time .*/
        ALM_RET_STOP,                               /* stop here. */
        ALM_RET_TOAL
        } alm_ret_enum;


    typedef struct alarm_manager_time_h {
        uint32_t        year;
        uint32_t        month;
        uint32_t        wday;
        uint32_t        day;
        uint32_t        hour;
        uint32_t        minute;
        uint32_t        second;
        } alarm_manager_time_t;


    typedef void(*on_alarm_expiring_t) (uint16_t index,uint32_t * user_data);
    void on_alarm_expiring(void * userData);
    alm_ret_enum add_alarm(uint16_t type,uint16_t index,alarm_manager_time_t * time,
        on_alarm_expiring_t on_alarm_expiring,uint32_t * user_data);
    alm_ret_enum cancel_alarm(uint16_t type,uint16_t index,alarm_manager_time_t * time,
        on_alarm_expiring_t on_alarm_expiring,uint32_t * user_data);
    void custom_set_alarm_time(aos_systime * time,void * cb,void * param);
    alm_ret_enum get_rtc_time_alarm(alarm_manager_time_t * time);
    void dt_increase_time(alarm_manager_time_t * timeToIncrement,alarm_manager_time_t * increment,
        alarm_manager_time_t * result);
    void custom_set_alarm_time_delay(uint32_t delay,uint32_t index,void * cb,void * param);
    void custom_set_calendar_time_delay(uint32_t delay,uint32_t index,void * cb,void * param);
    void custom_remove_alarm_by_number(uint8_t number);

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __SETTINGS_UI_H__ */

