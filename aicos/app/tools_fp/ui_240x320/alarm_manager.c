
#include "alarm_manager.h"
#include <rtc.h>
#include "list.h"
#include "app_log.h"

#define __inf                   APP_LOGI


typedef struct {
    alarm_manager_time_t time;
    uint16_t        type;
    uint16_t        index;
    on_alarm_expiring_t on_alarm_expiring;
    uint32_t *      user_data;
    struct list_head node;
} alarm_info_t;

typedef struct {
    struct list_head head;
} alarm_data_t;

#define DT_UTC_BASE_YEAR        1970
#define DT_MONTH_PER_YEAR       12
#define DT_DAY_PER_YEAR         365
#define DT_SEC_PER_DAY          86400
#define DT_SEC_PER_HOUR         3600
#define DT_SEC_PER_MIN          60
#define DT_MIN_PER_HR           60
#define DT_HRS_PRE_DAY          24
#define DT_MTH_PER_YEAR         12
#define DT_MAX_YEAR             2030
#define DT_MIN_YEAR             1970


const short     DAY_PER_MONTH[] =
{
    0,31,28,31,30,31,30,31,31,30,31,30,31
};


short dt_is_leap_year(short year)
{
    if ((year % 400) == 0) {
        return 1;
        }
    else if ((year % 100) == 0) {
        return 0;
        }
    else if ((year % 4) == 0) {
        return 1;
        }
    else {
        return 0;
        }
}


short dt_dow(short y,short m,short d)
{
    if (m < 3) {
        m                   += 13;
        y--;
        }
    else {
        m++;
        }

    return (d + 26 * m / 10 + y + y / 4 - y / 100 + y / 400 + 6) % 7;
}


short dt_last_day_of_mon(short month,short year)
{
    if ((month == 0) || (month > 12)) {
        return DAY_PER_MONTH[2] +dt_is_leap_year(year);
        }

    if (month != 2) {
        return DAY_PER_MONTH[month];
        }
    else {
        return DAY_PER_MONTH[2] +dt_is_leap_year(year);
        }
}


void dt_increase_time(alarm_manager_time_t * timeToIncrement,alarm_manager_time_t * increment,
    alarm_manager_time_t * result)
{
    int16_t         NumOfDays;

    /* second */
    result->second      = (timeToIncrement->second + increment->second);

    /* minute */
    result->minute      = (timeToIncrement->minute + increment->minute);

    if (result->second >= DT_MIN_PER_HR) {
        result->second      -= DT_MIN_PER_HR;
        result->minute      += 1;
        }

    /* hour */
    result->hour        = (timeToIncrement->hour + increment->hour);

    if (result->minute >= DT_MIN_PER_HR) {
        result->minute      -= DT_MIN_PER_HR;
        result->hour        += 1;
        }

    /* day */
    result->day         = (timeToIncrement->day + increment->day);

    if (result->hour >= DT_HRS_PRE_DAY) {
        result->hour        -= DT_HRS_PRE_DAY;
        result->day         += 1;
        }

    /* month, year */
    result->month       = timeToIncrement->month + increment->month;
    result->year        = timeToIncrement->year + increment->year;

    if (result->month > DT_MONTH_PER_YEAR) {
        result->month       -= DT_MONTH_PER_YEAR;
        result->year++;
        }

    /* correct day */
    NumOfDays           = dt_last_day_of_mon(timeToIncrement->month,timeToIncrement->year);

    if (result->day > NumOfDays) {
        result->day         -= NumOfDays;

        if (result->month != 12) {
            result->month       += 1;
            }
        else {
            result->month       = 1;
            result->year        += 1;
            }
        }

    result->wday        = dt_dow(result->year + DT_MIN_YEAR,result->month,result->day);
}


#define DT_TIME_LESS            -1
#define DT_TIME_EQUAL           0
#define DT_TIME_GREATER         1


short dt_compare_time(const alarm_manager_time_t * t1,const alarm_manager_time_t * t2)
{
    if (t1->year == t2->year) {
        if (t1->month == t2->month) {
            if (t1->day == t2->day) {
                if (t1->hour == t2->hour) {
                    if (t1->minute == t2->minute) {
                        if (t1->second == t2->second) {
                            return DT_TIME_EQUAL;
                            }
                        else {
                            if (t1->second > t2->second) {
                                return DT_TIME_GREATER;
                                }
                            else {
                                return DT_TIME_LESS;
                                }
                            }
                        }
                    else {
                        if (t1->minute > t2->minute) {
                            return DT_TIME_GREATER;
                            }
                        else {
                            return DT_TIME_LESS;
                            }
                        }
                    }
                else {
                    if (t1->hour > t2->hour) {
                        return DT_TIME_GREATER;
                        }
                    else {
                        return DT_TIME_LESS;
                        }
                    }
                }
            else {
                if (t1->day > t2->day) {
                    return DT_TIME_GREATER;
                    }
                else {
                    return DT_TIME_LESS;
                    }
                }
            }
        else {
            if (t1->month > t2->month) {
                return DT_TIME_GREATER;
                }
            else {
                return DT_TIME_LESS;
                }
            }
        }
    else {
        if (t1->year > t2->year) {
            return DT_TIME_GREATER;
            }
        else {
            return DT_TIME_LESS;
            }
        }

    return DT_TIME_EQUAL;
}


static alarm_data_t alarm_data;


void alarm_data_init()
{
    list_head_init(&alarm_data.head);
}


alm_ret_enum get_rtc_time_alarm(alarm_manager_time_t * time)
{
    aos_systime     cur_time;

    aos_get_systime(&cur_time);

    time->year          = cur_time.year;
    time->month         = cur_time.month;
    time->wday          = cur_time.wday;
    time->day           = cur_time.day;
    time->hour          = cur_time.hour;
    time->minute        = cur_time.minute;
    time->second        = cur_time.second;

    return ALM_RET_OK;

}


static alm_ret_enum set_rtc_alarm(alarm_manager_time_t * time)
{
    aos_systime     alarm_time;

    alarm_time.year     = time->year;
    alarm_time.month    = time->month;
    alarm_time.wday     = time->wday;
    alarm_time.day      = time->day;
    alarm_time.hour     = time->hour;
    alarm_time.minute   = time->minute;
    alarm_time.second   = time->second;
    alarm_time.msecond  = 0;
    alarm_time.usecond  = 0;


    aos_set_alarm_time(&alarm_time,on_alarm_expiring,(void *) NULL);

    return ALM_RET_OK;
}


alm_ret_enum add_queue_alarm(alarm_info_t * info)
{
    alarm_info_t *  alarm_info,*alarm_pre,*alarm_cur;

    struct list_head * pos,*next;
    short           cmp;

    alarm_info          = lv_mem_alloc(sizeof(alarm_info_t));
    memcpy(alarm_info,info,sizeof(alarm_info_t));

    alarm_pre           = NULL;
    list_for_each_safe(pos,next,&alarm_data.head) {
        alarm_cur           = list_entry(pos,alarm_info_t,node);
        cmp                 = dt_compare_time(&alarm_cur->time,&alarm_info->time);

        if (DT_TIME_LESS != cmp) {
            break;
            }

        alarm_pre           = alarm_cur;
        }

    if (alarm_pre == NULL)
        list_add(&alarm_info->node,&alarm_data.head);
    else 
        list_add(&alarm_info->node,&alarm_pre->node);


    return ALM_RET_OK;

}


alm_ret_enum remove_queue_alarm(alarm_info_t * info)
{
    alarm_info_t *  alarm_info;

    struct list_head * pos,*next;
    list_for_each_safe(pos,next,&alarm_data.head) {
        alarm_info          = list_entry(pos,alarm_info_t,node);

        if ((alarm_info->type == info->type) && (alarm_info->index == info->index) &&
             (alarm_info->on_alarm_expiring == info->on_alarm_expiring) &&
             (alarm_info->user_data == info->user_data)) {
            list_del(pos);
            lv_mem_free(alarm_info);
            break;
            }
        }
    return ALM_RET_OK;
}

alm_ret_enum remove_queue_alarm_by_number(alarm_info_t * info)
{
    alarm_info_t *  alarm_info;

    struct list_head * pos,*next;
    list_for_each_safe(pos,next,&alarm_data.head) {
        alarm_info          = list_entry(pos,alarm_info_t,node);

        if ((alarm_info->type == info->type) &&
             (* (uint8_t *) alarm_info->user_data == * (uint8_t *) info->user_data)) {
            __inf("alarm_info->index%d\r\n",alarm_info->index);
            list_del(pos);
            lv_mem_free(alarm_info);

            //			break;
            }
        }
    return ALM_RET_OK;
}

alm_ret_enum do_alarm_schedule(void)
{
    alarm_manager_time_t curr_time;
    alarm_info_t *  alarm_info;

    struct list_head * pos,*next;
    short           cmp;

    get_rtc_time_alarm(&curr_time);

    list_for_each_safe(pos,next,&alarm_data.head) {
        alarm_info          = list_entry(pos,alarm_info_t,node);

        cmp                 = dt_compare_time(&alarm_info->time,&curr_time);

        if (cmp == DT_TIME_GREATER) {
            set_rtc_alarm(&alarm_info->time);
            return ALM_RET_OK;
            }


        }

    if (list_empty(&alarm_data.head))
        aos_del_alarm_time();

    return ALM_RET_STOP;
}


alm_ret_enum add_alarm(uint16_t type,uint16_t index,alarm_manager_time_t * time,
    on_alarm_expiring_t on_alarm_expiring,uint32_t * user_data)
{
    alm_ret_enum    ret;
    alarm_manager_time_t curr_time;
    alarm_info_t    alarm_info;

    if (ALM_RET_OK != get_rtc_time_alarm(&curr_time)) {
        __inf("add_alarm:ALM_RET_FAIL \r\n");
        return ALM_RET_FAIL;
        }

    //__inf("add_alarm time:%d,%d,%d,%d,%d,%d,%d",time->year,time->month,time->wday,time->day,time->hour,time->minute,time->second);
    //__inf("add_alarm curr_time:%d,%d,%d,%d,%d,%d,%d",curr_time.year,curr_time.month,curr_time.wday,curr_time.day,curr_time.hour,curr_time.minute,curr_time.second);
    if (DT_TIME_GREATER == dt_compare_time(&curr_time,time)) {

        __inf("add_alarm:ALM_RET_INVALID_TIME \r\n");
        return ALM_RET_INVALID_TIME;
        }


    alarm_info.type     = type;
    alarm_info.index    = index;
    alarm_info.on_alarm_expiring = on_alarm_expiring;
    alarm_info.user_data = user_data;
    memcpy(&alarm_info.time,time,sizeof(alarm_manager_time_t));

    ret                 = remove_queue_alarm(&alarm_info);
    ret                 = add_queue_alarm(&alarm_info);

    if (ALM_RET_OK == ret) {
        do_alarm_schedule();
        }

    return ret;

}


alm_ret_enum cancel_alarm(uint16_t type,uint16_t index,alarm_manager_time_t * time,
    on_alarm_expiring_t on_alarm_expiring,uint32_t * user_data)
{
    alm_ret_enum    ret = ALM_RET_FAIL;

    alarm_info_t    alarm_info;

    if (type == ALM_TYPE_NONE || type >= ALM_TYPE_MAX) {
        __inf("cancel_alarm, ALM_RET_INVALID_TYPE \r\n");
        return ALM_RET_INVALID_TYPE;
        }


    alarm_info.type     = type;
    alarm_info.index    = index;
    alarm_info.on_alarm_expiring = on_alarm_expiring;
    alarm_info.user_data = user_data;
    memcpy(&alarm_info.time,time,sizeof(alarm_manager_time_t));
    ret                 = remove_queue_alarm(&alarm_info);

    if (ret == ALM_RET_OK) {
        do_alarm_schedule();
        }

    return ret;
}

alm_ret_enum remove_alarm_by_number(uint16_t type,uint8_t alarm_number)
{
    alm_ret_enum    ret = ALM_RET_FAIL;

    alarm_info_t    alarm_info;
    uint8_t         alarm_num = alarm_number;

    if (type == ALM_TYPE_NONE || type >= ALM_TYPE_MAX) {
        __inf("cancel_alarm, ALM_RET_INVALID_TYPE \r\n");
        return ALM_RET_INVALID_TYPE;
        }


    alarm_info.type     = type;
    alarm_info.index    = 0;
    alarm_info.on_alarm_expiring = NULL;
    alarm_info.user_data = (uint32_t *)&alarm_num;
    ret                 = remove_queue_alarm_by_number(&alarm_info);

    if (ret == ALM_RET_OK) {
        do_alarm_schedule();
        }

    return ret;
}

void on_alarm_expiring(void * userData)
{
    alarm_manager_time_t curr_time;
    alarm_info_t *  alarm_info;

    struct list_head * pos,*next;
    short           cmp;

    if (ALM_RET_OK != get_rtc_time_alarm(&curr_time)) {
        __inf("get_alarm:ALM_RET_FAIL \r\n");
        return;
        }

    list_for_each_safe(pos,next,&alarm_data.head) {
        alarm_info          = list_entry(pos,alarm_info_t,node);

        //__inf("add_alarm time1:%d,%d,%d,%d,%d,%d,%d",curr_time.year,curr_time.month,curr_time.wday,curr_time.day,curr_time.hour,curr_time.minute,curr_time.second);
        //__inf("add_alarm time2:%d,%d,%d,%d,%d,%d,%d",alarm_info->time.year,alarm_info->time.month,alarm_info->time.wday,alarm_info->time.day,alarm_info->time.hour,alarm_info->time.minute,alarm_info->time.second);
        cmp                 = dt_compare_time(&curr_time,&alarm_info->time);

        if (DT_TIME_LESS != cmp) {
            list_del(pos);

            if (alarm_info->on_alarm_expiring)
                alarm_info->on_alarm_expiring(alarm_info->index,alarm_info->user_data);

            lv_mem_free(alarm_info);
            }

        //__inf("add_alarm cmp :%d",cmp);
        }
    __inf("on_alarm_expiring");
    do_alarm_schedule();
}


void custom_set_alarm_time(aos_systime * time,void * cb,void * param)
{
    aos_systime     alarm_time;

    alarm_time.year     = time->year;
    alarm_time.month    = time->month;
    alarm_time.wday     = time->wday;
    alarm_time.day      = time->day;
    alarm_time.hour     = time->hour;
    alarm_time.minute   = time->minute;
    alarm_time.second   = time->second;


    add_alarm(ALM_TYPE_ALARM,0,(alarm_manager_time_t *) &alarm_time,cb,param);
}

void custom_set_alarm_time_delay(uint32_t delay,uint32_t index,void * cb,void * param)
{
    alarm_manager_time_t cur_time,incrase_time,alarm_time;

    get_rtc_time_alarm(&cur_time);
    memset(&incrase_time,0,sizeof(alarm_manager_time_t));
    cur_time.second     = 0;
    incrase_time.minute = delay;
    dt_increase_time(&cur_time,&incrase_time,&alarm_time);

    add_alarm(ALM_TYPE_ALARM,1 + index,&alarm_time,cb,param);
}

void custom_set_calendar_time_delay(uint32_t delay,uint32_t index,void * cb,void * param)
{
    alarm_manager_time_t cur_time,incrase_time,alarm_time;



    get_rtc_time_alarm(&cur_time);
    memset(&incrase_time,0,sizeof(alarm_manager_time_t));
    cur_time.second     = 0;
    incrase_time.minute = delay;
    dt_increase_time(&cur_time,&incrase_time,&alarm_time);



    add_alarm(ALM_TYPE_ALARM,1 + index,&alarm_time,cb,param);
}

void custom_remove_alarm_by_number(uint8_t number)
{
    remove_alarm_by_number(ALM_TYPE_ALARM,number);
}

