

/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of tools app ui.
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include "alv_app_mgr.h"
#include "aic_md_id.h"
#include <rtc.h>
#include "app_log.h"

#include "win_toolsmenu/ui.h"
#include "win_toolscalculator/ui.h"
#include "win_toolsalarm/ui.h"
#include "win_toolscalendar/ui.h"
#include "win_tooltts/ui.h"

#include "settings_fp_flow.h"
#include "backlight.h"
#include "app_msg.h"
#include "aic_md_api.h"
#include "aic_boot_mode.h"
#include "tts_app_player.h"
#include "alarm_manager.h"
#include "tools_ui.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/
uint8_t         alarm_doing_number = 0;
aos_systime     copy_alarm_time;
signed char     alarm_para;
signed char     copy_alarm_para;
uint8_t         g_alarm_para = 0xff;
uint8_t         shortcut_in = 0;

typedef struct _TOOLS_ALARM_TIME_STATE_PARAM_H_ {
    uint8_t         state;                          // =0闹钟关闭；=1闹钟打开
    uint8_t         alarm_type;                     // =0,一次性闹钟；=1,重复闹钟
    uint8_t         alarm_sign;                     //bit0-bit6表示周日到周六的重复闹钟（bit0代表周日）；bit7是一次性闹钟的标记，每位=1表示需要闹铃，=0不需要闹；一次性闹钟闹铃后把BIT7清除掉
    uint8_t         hour;
    uint8_t         min;
} toolsalarm_time_state_param_t;

typedef struct _TOOLS_ALARM_STATE_PARAM_H_ {
    uint8_t         ring;
    toolsalarm_time_state_param_t alarm_item[3];
    uint8_t         level;
    uint8_t         reserve[3];
} toolsalarm_state_param_t;

typedef struct _TODOLIST_NVRAM_PARAM_H_ {
    uint16_t        year;
    uint8_t         month;
    uint8_t         day;
    uint8_t         hour;
    uint8_t         min;
} todolist_nvram_param_t;

typedef struct _POWER_ON_ALARM_STATE_PARAM_H_ {
    uint8_t         alarm_id;
    uint8_t         reserve[3];
} poweron_alarm_state_param_t;

typedef struct _TODOLIST_ALL_NVRAM_PARAM_H_ {
    todolist_nvram_param_t todolist[10];            // 6*10 = 60,// 4字节对齐
} todolist_all_nvram_param_t;

void alarm_para_init(void)
{
    g_alarm_para        = 0xff;
    alarm_para          = 0xff;
    copy_alarm_para     = 0xff;
    aos_memset((u8 *) &copy_alarm_time,0,sizeof(aos_systime));
}


extern uint8_t get_alarm_ring(void);


static int32_t alarm_doing_show_action(void * p_param,uint32_t size)
{
    uint8_t *       p_alarm_info = p_param;

    //APP_LOGI("alarm_doing_show_action\n");
    //aic_delay(1);
    if (alv_am_app_is_exist("call")) {
        alarm_doing_number  = * (uint8_t *) p_alarm_info;
        active_recent_alarm(0);

        //active_alarm_delay(alarm_doing_number);
        return - 1;
        }

    if (false == alv_am_app_is_exist("tools")) {
        alv_am_app_enter_action(NULL,"tools",p_alarm_info);
        }
    else {
        char            name[32];

        memset((u8 *) name,0,32);
        alv_am_win_get_cur_winname(name);

        if ((0 == strcmp(name,"tool_alarm_doing")) || (0 == strcmp(name,"tool_cal_doing"))) {
            toolsalarm_state_param_t alarm_state;
            s32             ret;
            uint8_t         alarm_number;

            alarm_number        = * (uint8_t *) p_alarm_info;

            if (alarm_number <= 2) {
                ret                 = aic_md_read(MD_ID_TOOLS_ALARM,sizeof(toolsalarm_state_param_t),&alarm_state);

                if (ret) {
                    APP_LOGI("read net cfg error 0x%x\n",ret);
                    }

                if (alarm_state.alarm_item[alarm_number].alarm_type == 0) //单次闹钟
                {
                    alarm_state.alarm_item[alarm_number].state = 0; //单次闹钟一次后关掉
                    alarm_state.alarm_item[alarm_number].alarm_sign = 0; //
                    ret                 = aic_md_write(MD_ID_TOOLS_ALARM,sizeof(toolsalarm_state_param_t),
                        &alarm_state);

                    if (ret) {
                        APP_LOGI("write net cfg error 0x%x\n",ret);
                        }
                    }
                }

            return 0;
            }

        alarm_doing_number  = * (uint8_t *) p_alarm_info;
        APP_LOGI("alarm_doing_show_action 11=%d\n",alarm_doing_number);

        if (alarm_doing_number <= 2)
            alv_am_win_jump_action(NULL,"tool_alarm_doing",NULL);
        else if (alarm_doing_number < 3 + 10) {
            alarm_doing_number  -= 3;
            alv_am_win_jump_action(NULL,"tool_cal_doing",NULL);
            }
        else 
            return 0;
        }

    return 0;
}


void alarm_doing_show(uint8_t * p_alarm_info)
{
    APP_LOGI("alarm_doing_show\n");

    app_msg_send_callback(alarm_doing_show_action,p_alarm_info,sizeof(uint8_t));
}


void alarm_doing_show1(uint16_t index,uint8_t * p_alarm_info)
{
    APP_LOGI("alarm_doing_show1\n");

    app_msg_send_callback(alarm_doing_show_action,p_alarm_info,sizeof(uint8_t));
}


void reset_power_on_alarm_id(void)
{
    s32             ret;
    poweron_alarm_state_param_t power_on_alarm;

    power_on_alarm.alarm_id = 0xff;
    ret                 = aic_md_write(MD_ID_ALARM_DOING_ID,sizeof(poweron_alarm_state_param_t),&power_on_alarm);

    if (ret) {
        APP_LOGI("read power_on_alarm cfg error 0x%x\n",ret);
        }
}


void power_on_alarm(void)
{
    s32             ret;
    poweron_alarm_state_param_t power_on_alarm;

    APP_LOGI("power_on_alarm %d\n",aic_boot_mode_get());

    if (aic_boot_reason_get() != 6) //BOOT_REASON_ALARM_RTC
        return;

    ret                 = aic_md_read(MD_ID_ALARM_DOING_ID,sizeof(poweron_alarm_state_param_t),&power_on_alarm);

    if (ret) {
        APP_LOGI("read power_on_alarm cfg error 0x%x\n",ret);
        }

    if (power_on_alarm.alarm_id > 13) //超过13就是错误的
        return;

    g_alarm_para        = power_on_alarm.alarm_id;
    alarm_doing_show(&g_alarm_para);
}


boolean is_leap_year(u32 year)
{
    return (year % 4) || ((year % 100 == 0) && (year % 400)) ? 0: 1;
}


#define DELAY_TIME              10

//同时检查闹钟和备忘录，把最近的一个闹钟设置为有效;type =0,
// 重新设置闹钟,已经设置的闹钟时间不加10，type=1或2，3，已经设置的闹钟加10分钟
void active_recent_alarm(uint8_t type)
{
    todolist_all_nvram_param_t todolist_all;
    toolsalarm_state_param_t tools_state;
    aos_systime     alarm_time,rtc_time1;
    s32             ret;
    uint8_t         date[4];
    uint8_t         time[2];
    uint8_t         min;

    //	APP_LOGI("active_recent_alarm type =%d\n",type);
    aos_memset((u8 *) &alarm_time,0,sizeof(aos_systime));
    aos_memset((u8 *) &rtc_time1,0,sizeof(aos_systime));
    alarm_para          = gx_rtc_alarm_param();

    if ((alarm_para < 0) || (type == 4)) {
        alarm_time.year     = 0;
        }
    else {
        aos_get_alarm_time(&alarm_time);
        }

    if (type && (type != 4)) {
        if (type == 3) {
            settings_flow_get_datetime_dec(date,time);
            alarm_time.year     = date[0] +1970;
            alarm_time.month    = date[1];
            alarm_time.day      = date[2];
            alarm_time.wday     = date[3];
            alarm_time.hour     = time[0];
            alarm_time.minute   = time[1];
            alarm_time.second   = 0;
            alarm_time.msecond  = 0;
            alarm_time.usecond  = 0;
            min                 = alarm_time.minute + DELAY_TIME;
            }
        else {
            min                 = alarm_time.minute + DELAY_TIME + type - 1; //手动按键延时只加10分钟，一分钟到了自动延时加11分钟
            }

        if (min >= 60) {
            alarm_time.minute   = min - 60;

            if (alarm_time.hour + 1 >= 24) {
                alarm_time.hour     = alarm_time.hour + 1 - 24;

                if (alarm_time.month == 2 && is_leap_year(alarm_time.year)) {
                    if (alarm_time.day + 1 > 29) {
                        alarm_time.day      = 1;
                        alarm_time.month    = 3;
                        }
                    else 
                        alarm_time.day += 1;
                    }
                else if (alarm_time.month == 2) {
                    if (alarm_time.day + 1 > 28) {
                        alarm_time.day      = 1;
                        alarm_time.month    = 3;
                        }
                    else 
                        alarm_time.day += 1;
                    }
                else if (alarm_time.month == 12) {
                    if (alarm_time.day + 1 > 31) {
                        alarm_time.day      = 1;
                        alarm_time.month    = 1;
                        alarm_time.year     += 1;
                        }
                    else 
                        alarm_time.day += 1;
                    }
                else 
                    alarm_time.day += 1;
                }
            else {
                alarm_time.hour     += 1;
                }
            }
        else 
            alarm_time.minute = min;

        alarm_time.wday     = get_wday(alarm_time.year,alarm_time.month,alarm_time.day);

        }
    else {
        if (alarm_time.year == 0) {
            alarm_time.year     = 2038;             // 2037 + 1
            alarm_time.month    = 12;
            alarm_time.day      = 31;
            alarm_time.wday     = 0;
            alarm_time.hour     = 0;
            alarm_time.minute   = 0;
            alarm_time.second   = 0;
            alarm_time.usecond  = 0;
            alarm_time.msecond  = 0;
            }
        }

    settings_flow_get_datetime_dec(date,time);
    rtc_time1.year      = date[0] +1970;
    rtc_time1.month     = date[1];
    rtc_time1.day       = date[2];
    rtc_time1.wday      = date[3];
    rtc_time1.hour      = time[0];
    rtc_time1.minute    = time[1];
    rtc_time1.second    = 0;
    rtc_time1.msecond   = 0;
    rtc_time1.usecond   = 0;
    ret                 = aic_md_read(MD_ID_TOOLS_ALARM,sizeof(toolsalarm_state_param_t),&tools_state);

    if (ret) {
        APP_LOGI("read net cfg error 0x%x\n",ret);
        }

    ret                 = aic_md_read(MD_ID_TOOLS_TODO,sizeof(todolist_all_nvram_param_t),&todolist_all);

    if (ret) {
        APP_LOGI("read net cfg error 0x%x\n",ret);
        }

    u32             alarm_recent_time[3];
    u32             alarm_delay_day[3];
    u32             time0,time1;

    time0               =
         rtc_tm_to_days((struct rtc_time *) &rtc_time1) * 24 * 60 + rtc_time1.hour * 60 + rtc_time1.minute; //计算某个确定日期到1970的相对分钟数

    for (uint8_t i = 0; i < 3; i++) {
        alarm_delay_day[i]  = 0;

        if (tools_state.alarm_item[i].state) {
            aos_systime     current_time;

            current_time.year   = date[0] +1970;
            current_time.month  = date[1];
            current_time.day    = date[2];
            current_time.wday   = date[3];
            current_time.hour   = tools_state.alarm_item[i].hour;
            current_time.minute = tools_state.alarm_item[i].min;
            current_time.second = 0;
            current_time.msecond = 0;
            current_time.usecond = 0;
            time1               =
                 rtc_tm_to_days((struct rtc_time *) &current_time) * 24 * 60 + current_time.hour * 60 + current_time.minute;

            //计算某个确定日期到1970的相对分钟数
            if (tools_state.alarm_item[i].alarm_type == 0) {
                if (time1 > time0) {
                    alarm_recent_time[i] = time1 - time0;

                    //					APP_LOGI("active_recent_alarm aaa =%d,%d\n",i,alarm_recent_time[i]);
                    continue;
                    }
                else {
                    alarm_delay_day[i]  = 1;

                    if (current_time.month == 2 && is_leap_year(current_time.year)) {
                        if (current_time.day + 1 > 29) {
                            current_time.day    = 1;
                            current_time.month  = 3;
                            }
                        else 
                            current_time.day += 1;
                        }
                    else if (current_time.month == 12) {
                        if (current_time.day + 1 > 31) {
                            current_time.day    = 1;
                            current_time.month  = 1;
                            current_time.year   += 1;
                            }
                        else 
                            current_time.day += 1;
                        }
                    else {
                        uint8_t         month_days[13] = {
                            0,31,28,31,30,31,30,31,31,30,31,30,31
                            };

                        if (current_time.day + 1 > month_days[current_time.month]) {
                            current_time.day    = 1;
                            current_time.month  += 1;
                            }
                        else 
                            current_time.day += 1;
                        }

                    time1               =
                         rtc_tm_to_days((struct rtc_time *) &current_time) * 24 * 60 + current_time.hour * 60 + current_time.minute;

                    //计算某个确定日期到1970的相对分钟数
                    alarm_recent_time[i] = time1 - time0;

                    //					APP_LOGI("active_recent_alarm bbb =%d,%d\n",i,alarm_recent_time[i]);
                    continue;
                    }
                }
            else {
                uint8_t         days = 0;
                uint8_t         days_array[7] = {
                    0x01,0x02,0x04,0x08,0x10,0x20,0x40
                    };

                for (uint8_t j = rtc_time1.wday; j < 7; j++) {
                    if (tools_state.alarm_item[i].alarm_sign & days_array[j]) {
                        days                = j - rtc_time1.wday;

                        if ((days * 24 * 60 + tools_state.alarm_item[i].hour * 60 + tools_state.alarm_item[i].min) >
                             (time[0] *60 + time[1])) {
                            //days = j - rtc_time1.wday;
                            alarm_delay_day[i]  = days;
                            goto alarm_out;
                            }
                        }
                    }

                for (uint8_t j = 0; j < rtc_time1.wday; j++) {
                    if (tools_state.alarm_item[i].alarm_sign & days_array[j]) {
                        days                = 7 + j - rtc_time1.wday;
                        alarm_delay_day[i]  = days;
                        goto alarm_out;
                        }
                    }








alarm_out:
                time1               = time1 + days * 24 * 60;

                //				if(time1 < time0)				//同一天，且闹钟时间小于实时时间，那就是到下周的今天才响闹钟
                //					time1 = time1 + 7 * 24 * 60;
                alarm_recent_time[i] = time1 - time0;

                }

            }
        else 
            alarm_recent_time[i] = 0xffffffff;

        }

    u32             todo_recent_time[10];

    for (uint8_t i = 0; i < 10; i++) {
        if (todolist_all.todolist[i].year != 0 && todolist_all.todolist[i].hour != 0xff &&
             todolist_all.todolist[i].min != 0xff) {
            struct rtc_time cal_time;
            cal_time.tm_year    = todolist_all.todolist[i].year;
            cal_time.tm_mon     = todolist_all.todolist[i].month;
            cal_time.tm_day     = todolist_all.todolist[i].day;
            cal_time.tm_hour    = todolist_all.todolist[i].hour;
            cal_time.tm_min     = todolist_all.todolist[i].min;
            cal_time.tm_sec     = 0;
            cal_time.tm_msec    = 0;
            cal_time.tm_usec    = 0;
            time1               =
                 rtc_tm_to_days(&cal_time) * 24 * 60 + todolist_all.todolist[i].hour * 60 + todolist_all.todolist[i].min;

            //计算某个确定日期到1970的相对分钟数
            if (time1 > time0) {
                todo_recent_time[i] = time1 - time0;
                }
            else 
                todo_recent_time[i] = 0xffffffff;
            }
        else 
            todo_recent_time[i] = 0xffffffff;
        }

    if (copy_alarm_time.year >= 1970 && copy_alarm_time.year <= 2037) { //
        time1               =
             rtc_tm_to_days((struct rtc_time *) &copy_alarm_time) * 24 * 60 + copy_alarm_time.hour * 60 + copy_alarm_time.minute;

        //计算某个确定日期到1970的相对分钟数
        }
    else {
        time1               =
             rtc_tm_to_days((struct rtc_time *) &alarm_time) * 24 * 60 + alarm_time.hour * 60 + alarm_time.minute; //计算某个确定日期到1970的相对分钟数
        }

    u32             time2 = time1 - time0;
    uint8_t         index1 = 0xff;
    uint8_t         index2 = 0xff;

    APP_LOGI("active_recent_alarm ccc copy_alarm_time.year=%d,alarm_time.year=%d,time2=%d\n",copy_alarm_time.year,
        alarm_time.year,time2);

    for (uint8_t i = 0; i < 3; i++) {
        APP_LOGI("active_recent_alarm cccc00 =%d,%d,%d\n",i,alarm_recent_time[i],time2);

        if (time2 > alarm_recent_time[i]) {
            time2               = alarm_recent_time[i];
            index1              = i;
            }
        }

    for (uint8_t i = 0; i < 10; i++) {

        APP_LOGI("active_recent_alarm cccc111 =%d,%d,%d\n",i,todo_recent_time[i],time2);

        if (time2 > todo_recent_time[i]) {
            time2               = todo_recent_time[i];
            index2              = i;
            }
        }

    if (index2 != 0xff) { // 备忘录作为闹钟

        aos_systime     todo_time;

        todo_time.year      = todolist_all.todolist[index2].year;
        todo_time.month     = todolist_all.todolist[index2].month;
        todo_time.day       = todolist_all.todolist[index2].day;
        todo_time.wday      = get_wday(todo_time.year,todo_time.month,todo_time.day);
        todo_time.hour      = todolist_all.todolist[index2].hour;
        todo_time.minute    = todolist_all.todolist[index2].min;
        todo_time.second    = 0;
        todo_time.msecond   = 0;
        todo_time.usecond   = 0;
        index2              += 3;                   //索引号加3，因为闹钟和备忘录统一编索引号
        alarm_para          = gx_rtc_alarm_param();

        if ((type && (type != 4)) || (((alarm_time.year >= 1970) && (alarm_time.year <= 2037)) &&
             (alarm_para != index2))) {
            aos_memset((u8 *) &copy_alarm_time,0,sizeof(aos_systime));
            memcpy(&copy_alarm_time,&alarm_time,sizeof(aos_systime));
            copy_alarm_para     = index2;

            APP_LOGI("active_recent_alarm 11 00 alarm_para=%d,year =%d\n",alarm_para,copy_alarm_time.year);
            }
        else 
            aos_memset((u8 *) &copy_alarm_time,0,sizeof(aos_systime));

        APP_LOGI("active_recent_alarm ddd =%d\n",index2);
        g_alarm_para        = index2;
        custom_set_alarm_time(&todo_time,alarm_doing_show1,(void *) &g_alarm_para);
        s32             ret;
        poweron_alarm_state_param_t power_on_alarm;

        power_on_alarm.alarm_id = g_alarm_para;
        ret                 = aic_md_write(MD_ID_ALARM_DOING_ID,sizeof(poweron_alarm_state_param_t),&power_on_alarm);

        if (ret) {
            APP_LOGI("read power_on_alarm cfg error 0x%x\n",ret);
            }

        }
    else if (index1 != 0xff) {
        aos_systime     current_time;
        uint8_t         month_days[13] = {
            0,31,28,31,30,31,30,31,31,30,31,30,31
            };
        uint8_t         days = 0;

        days                = alarm_delay_day[index1];
        current_time.year   = date[0] +1970;
        current_time.month  = date[1];
        current_time.day    = date[2];
        current_time.hour   = tools_state.alarm_item[index1].hour;
        current_time.minute = tools_state.alarm_item[index1].min;
        current_time.second = 0;
        current_time.msecond = 0;
        current_time.usecond = 0;

        if (current_time.month == 2 && is_leap_year(current_time.year)) {
            if (current_time.day + days > 29) {
                current_time.day    = current_time.day + days - 29;
                current_time.month  = 3;
                }
            else 
                current_time.day += days;
            }
        else if (current_time.month == 12) {
            if (current_time.day + days > 31) {
                current_time.day    = current_time.day + days - 31;
                current_time.month  = 1;
                current_time.year   += 1;
                }
            else 
                current_time.day += days;
            }
        else if (current_time.day + days > month_days[current_time.month]) {
            current_time.day    = current_time.day + days - month_days[current_time.month];
            current_time.month  += 1;
            }
        else 
            current_time.day += days;

        current_time.wday   = get_wday(current_time.year,current_time.month,current_time.day);

        if (type) {
            alarm_para          = gx_rtc_alarm_param();

            if ((type && (type != 4)) || (((alarm_time.year >= 1970) && (alarm_time.year <= 2037)) &&
                 (alarm_para != index1))) {
                aos_memset((u8 *) &copy_alarm_time,0,sizeof(aos_systime));
                memcpy(&copy_alarm_time,&alarm_time,sizeof(aos_systime));
                copy_alarm_para     = alarm_para;

                //			APP_LOGI("active_recent_alarm eee 00 alarm_para=%d,copy_alarm_time.year =%d,month =%d,day =%d,hour =%d,minute =%d\n",alarm_para,copy_alarm_time.year,copy_alarm_time.month,copy_alarm_time.day,copy_alarm_time.hour,copy_alarm_time.minute);
                }
            else 
                aos_memset((u8 *) &copy_alarm_time,0,sizeof(aos_systime));

            //		APP_LOGI("active_recent_alarm 22 =%d\n",index1);
            char            str_date[30] = {
                0
                };

            //		APP_LOGI("active_recent_alarm fff year=%d,month=%d,day=%d,hour=%d,minute=%d\n",current_time.year,current_time.month, current_time.day,current_time.hour,current_time.minute);
            sprintf(str_date,"%04d/%02d/%02d/%02d/%02d",current_time.year,current_time.month,current_time.day,
                current_time.hour,current_time.minute);

            APP_LOGI("active_recent_alarm ggg =%s\n",str_date);
            g_alarm_para        = index1;

            custom_set_alarm_time(&current_time,alarm_doing_show1,(void *) &g_alarm_para);
            s32             ret;
            poweron_alarm_state_param_t power_on_alarm;

            power_on_alarm.alarm_id = g_alarm_para;
            ret                 = aic_md_write(MD_ID_ALARM_DOING_ID,sizeof(poweron_alarm_state_param_t),
                &power_on_alarm);

            if (ret) {
                APP_LOGI("read power_on_alarm cfg error 0x%x\n",ret);
                }

            }
        else {
            // 延时再响闹钟
            if (copy_alarm_time.year >= 1970 && copy_alarm_time.year <= 2037) //前一个延时闹钟时间更靠前，所以先有效
            {
                alarm_para          = gx_rtc_alarm_param();
                g_alarm_para        = copy_alarm_para;

                //			APP_LOGI("active_recent_alarm hhh copy_alarm_para=%d,year=%d,month=%d,day=%d,hour=%d,minute=%d\n",copy_alarm_para,copy_alarm_time.year,copy_alarm_time.month, copy_alarm_time.day,copy_alarm_time.hour,copy_alarm_time.minute);
                custom_set_alarm_time(&copy_alarm_time,alarm_doing_show1,(void *) &g_alarm_para);

                //			APP_LOGI("active_recent_alarm mmm =%d\n",alarm_para);
                aos_memset((u8 *) &copy_alarm_time,0,sizeof(aos_systime));

                if (alarm_time.year >= 1970 && alarm_time.year <= 2037) {
                    memcpy(&copy_alarm_time,&alarm_time,sizeof(aos_systime));
                    copy_alarm_para     = alarm_para;
                    }

                s32             ret;
                poweron_alarm_state_param_t power_on_alarm;

                power_on_alarm.alarm_id = g_alarm_para;
                ret                 = aic_md_write(MD_ID_ALARM_DOING_ID,sizeof(poweron_alarm_state_param_t),
                    &power_on_alarm);

                if (ret) {
                    APP_LOGI("read power_on_alarm cfg error 0x%x\n",ret);
                    }

                //			APP_LOGI("active_recent_alarm kkk copy_alarm_para =%d,year=%d,month=%d,day=%d,hour=%d,minute=%d\n",copy_alarm_para,copy_alarm_time.year,copy_alarm_time.month, copy_alarm_time.day,copy_alarm_time.hour,copy_alarm_time.minute);
                }
            else {
                aos_memset((u8 *) &copy_alarm_time,0,sizeof(aos_systime));
                alarm_para          = gx_rtc_alarm_param();

                if ((alarm_para < 0) || (type == 0))
                    return;

                g_alarm_para        = alarm_para;

                //			APP_LOGI("active_recent_alarm pppp year=%d,month=%d,day=%d,hour=%d,minute=%d\n",alarm_time.year,alarm_time.month, alarm_time.day,alarm_time.hour,alarm_time.minute);
                custom_set_alarm_time(&alarm_time,alarm_doing_show1,(void *) &g_alarm_para);

                //			APP_LOGI("active_recent_alarm 44 =%d\n",alarm_para);
                alarm_para          = 0xff;
                s32             ret;
                poweron_alarm_state_param_t power_on_alarm;

                power_on_alarm.alarm_id = g_alarm_para;
                ret                 = aic_md_write(MD_ID_ALARM_DOING_ID,sizeof(poweron_alarm_state_param_t),
                    &power_on_alarm);

                if (ret) {
                    APP_LOGI("read power_on_alarm cfg error 0x%x\n",ret);
                    }

                }
            }
        }
}

static uint8_t  alarm_user_data[14];

void active_alarm_delay(uint8_t type)
{
    alarm_user_data[type]     = type;
    custom_set_alarm_time_delay(10,type,alarm_doing_show1,(void *) &alarm_user_data[type]);
}

void active_calendar_delay(uint8_t type)
{
    alarm_user_data[type]     = type;
    custom_set_calendar_time_delay(10,type,alarm_doing_show1,(void *) &alarm_user_data[type]);
}

void tools_fp_app_active(lv_obj_t * app,void * p_param)
{
    lv_obj_t *      frist_win;

    APP_LOGI("tools_fp_app_active  =0x%x\n",p_param);

    //	aic_delay(1);
    if (p_param == NULL) {
        APP_LOGI("tools_fp_app_active11\n");

        frist_win           = get_win_by_name(app,"tools_menu");
        alv_am_app_set_first_win(app,frist_win);
        alarm_doing_number  = 0xff;
        return;
        }

    uint8_t         enter_param = * (uint8_t *) p_param;

    APP_LOGI("tools_fp_app_active22  =%d\n",enter_param);

    //	aic_delay(1);
    if (enter_param <= 2) {

        alarm_doing_number  = enter_param;
        frist_win           = get_win_by_name(app,"tool_alarm_doing");
        alv_am_app_set_first_win(app,frist_win);
        }
    else if (enter_param < 3 + 10) {
        alarm_doing_number  = enter_param - 3;
        frist_win           = get_win_by_name(app,"tool_cal_doing");
        alv_am_app_set_first_win(app,frist_win);
        }
}


void tools_fp_app_deactive(lv_obj_t * app,void * p_param)
{

}


void tts_option_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenTtsOption_screen_init(win,p_param);

}


void tts_option_fp_win_deactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenTtsOption_screen_uninit(win,p_param);
}


void tts_option_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenTtsOption_screen_focus(win,p_param);

}


void tts_option_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}


void tts_option_on_off_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenTtsOptionOnOff_screen_init(win,p_param);

}


void tts_option_on_off_fp_win_deactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenTtsOptionOnOff_screen_uninit(win,p_param);
}


void tts_option_on_off_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenTtsOptionOnOff_screen_focus(win,p_param);

}


void tts_option_on_off_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}


void tts_option_vol_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenTtsOptionVol_screen_init(win,p_param);

}


void tts_option_vol_fp_win_deactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenTtsOptionVol_screen_uninit(win,p_param);
}


void tts_option_vol_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenTtsOptionVol_screen_focus(win,p_param);

}


void tts_option_vol_fp_win_defocus(lv_obj_t * win,void * p_param)
{

    ui_ScreenTtsOptionVol_screen_defocus(win,p_param);

}


void tts_option_time_auto_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenTtsOptionTimeAuto_screen_init(win,p_param);

}


void tts_option_time_auto_fp_win_deactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenTtsOptionTimeAuto_screen_uninit(win,p_param);
}


void tts_option_time_auto_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenTtsOptionTimeAuto_screen_focus(win,p_param);

}


void tts_option_time_auto_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}


void tts_option_time_set_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenTtsOptionTimeSet_screen_init(win,p_param);

}


void tts_option_time_set_fp_win_deactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenTtsOptionTimeSet_screen_uninit(win,p_param);
}


void tts_option_time_set_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenTtsOptionTimeSet_screen_focus(win,p_param);

}


void tts_option_time_set_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}


void tts_option_time_select_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenTtsOptionTimeSelect_screen_init(win,p_param);

}


void tts_option_time_select_fp_win_deactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenTtsOptionTimeSelect_screen_uninit(win,p_param);
}


void tts_option_time_select_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenTtsOptionTimeSelect_screen_focus(win,p_param);

}


void tts_option_time_select_fp_win_defocus(lv_obj_t * win,void * p_param)
{


}


/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * settings_ui_init() - the ui init of tools app.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void tools_ui_init(void)
{
    /* Create APP for tools */
    lv_obj_t *      app_tools = alv_am_app_create(lv_scr_act(),"tools");

    alv_am_app_set_active(app_tools,tools_fp_app_active);
    alv_am_app_set_deactive(app_tools,tools_fp_app_deactive);

    lv_obj_t *      win_tools_menu = alv_am_win_create(app_tools,"tools_menu"); // 工具菜单本身

    alv_am_win_set_active(win_tools_menu,tools_win_active);
    alv_am_win_set_deactive(win_tools_menu,tools_win_deactive);
    alv_am_win_set_focus(win_tools_menu,tools_win_focus);
    alv_am_win_set_defocus(win_tools_menu,tools_win_defocus);
    alv_am_app_set_first_win(app_tools,win_tools_menu);

    //////////////////////////////////////////////////////////////////////////////////////////////////////	
    lv_obj_t *      win_tools_alarm = alv_am_win_create(app_tools,"tools_alarm"); // 闹钟

    alv_am_win_set_active(win_tools_alarm,tools_alarm_win_active);
    alv_am_win_set_deactive(win_tools_alarm,tools_alarm_win_deactive);
    alv_am_win_set_focus(win_tools_alarm,tools_alarm_win_focus);
    alv_am_win_set_defocus(win_tools_alarm,tools_alarm_win_defocus);
    lv_obj_t *      win_tools_alarm_ring = alv_am_win_create(app_tools,"tool_alarm_ring"); // 闹钟铃声

    alv_am_win_set_active(win_tools_alarm_ring,tools_alarm_ring_win_active);
    alv_am_win_set_deactive(win_tools_alarm_ring,tools_alarm_ring_win_deactive);
    alv_am_win_set_focus(win_tools_alarm_ring,tools_alarm_ring_win_focus);
    alv_am_win_set_defocus(win_tools_alarm_ring,tools_alarm_ring_win_defocus);
    lv_obj_t *      win_tools_alarm_item = alv_am_win_create(app_tools,"tool_alarm_item"); // 闹钟设置

    alv_am_win_set_active(win_tools_alarm_item,tools_alarm_item_win_active);
    alv_am_win_set_deactive(win_tools_alarm_item,tools_alarm_item_win_deactive);
    alv_am_win_set_focus(win_tools_alarm_item,tools_alarm_item_win_focus);
    alv_am_win_set_defocus(win_tools_alarm_item,tools_alarm_item_win_defocus);
    lv_obj_t *      win_tools_alarm_time = alv_am_win_create(app_tools,"tool_alarm_time"); // 闹钟设置

    alv_am_win_set_active(win_tools_alarm_time,tools_alarm_time_win_active);
    alv_am_win_set_deactive(win_tools_alarm_time,tools_alarm_time_win_deactive);
    alv_am_win_set_focus(win_tools_alarm_time,tools_alarm_time_win_focus);
    alv_am_win_set_defocus(win_tools_alarm_time,tools_alarm_time_win_defocus);
    lv_obj_t *      win_tools_alarm_set = alv_am_win_create(app_tools,"tool_alarm_set"); // 闹钟设置

    alv_am_win_set_active(win_tools_alarm_set,tools_alarm_set_win_active);
    alv_am_win_set_deactive(win_tools_alarm_set,tools_alarm_set_win_deactive);
    alv_am_win_set_focus(win_tools_alarm_set,tools_alarm_set_win_focus);
    alv_am_win_set_defocus(win_tools_alarm_set,tools_alarm_set_win_defocus);
    lv_obj_t *      win_tools_alarm_mode = alv_am_win_create(app_tools,"tool_alarm_mode"); // 闹钟设置

    alv_am_win_set_active(win_tools_alarm_mode,tools_alarm_mode_win_active);
    alv_am_win_set_deactive(win_tools_alarm_mode,tools_alarm_mode_win_deactive);
    alv_am_win_set_focus(win_tools_alarm_mode,tools_alarm_mode_win_focus);
    alv_am_win_set_defocus(win_tools_alarm_mode,tools_alarm_mode_win_defocus);
    lv_obj_t *      win_tools_alarm_sign = alv_am_win_create(app_tools,"tool_alarm_sign"); // 闹钟设置

    alv_am_win_set_active(win_tools_alarm_sign,tools_alarm_sign_win_active);
    alv_am_win_set_deactive(win_tools_alarm_sign,tools_alarm_sign_win_deactive);
    alv_am_win_set_focus(win_tools_alarm_sign,tools_alarm_sign_win_focus);
    alv_am_win_set_defocus(win_tools_alarm_sign,tools_alarm_sign_win_defocus);
    lv_obj_t *      win_tools_alarm_doing = alv_am_win_create(app_tools,"tool_alarm_doing"); // 闹钟设置

    alv_am_win_set_active(win_tools_alarm_doing,tools_alarm_doing_win_active);
    alv_am_win_set_deactive(win_tools_alarm_doing,tools_alarm_doing_win_deactive);
    alv_am_win_set_focus(win_tools_alarm_doing,tools_alarm_doing_win_focus);
    alv_am_win_set_defocus(win_tools_alarm_doing,tools_alarm_doing_win_defocus);

    //////////////////////////////////////////////////////////////////////////////////////////////////////	
    lv_obj_t *      win_tools_calculator = alv_am_win_create(app_tools,"tools_calculator"); // 计算器

    alv_am_win_set_active(win_tools_calculator,tools_calculator_win_active);
    alv_am_win_set_deactive(win_tools_calculator,tools_calculator_win_deactive);
    alv_am_win_set_focus(win_tools_calculator,tools_calculator_win_focus);
    alv_am_win_set_defocus(win_tools_calculator,tools_calculator_win_defocus);

    //////////////////////////////////////////////////////////////////////////////////////////////////////	
    lv_obj_t *      win_tools_calendar = alv_am_win_create(app_tools,"tools_calendar"); // 日历

    alv_am_win_set_active(win_tools_calendar,tools_calendar_win_active);
    alv_am_win_set_deactive(win_tools_calendar,tools_calendar_win_deactive);
    alv_am_win_set_focus(win_tools_calendar,tools_calendar_win_focus);
    alv_am_win_set_defocus(win_tools_calendar,tools_calendar_win_defocus);
    lv_obj_t *      win_tools_calendar_item = alv_am_win_create(app_tools,"tool_cal_item"); // 日历

    alv_am_win_set_active(win_tools_calendar_item,tools_calendar_item_win_active);
    alv_am_win_set_deactive(win_tools_calendar_item,tools_calendar_item_win_deactive);
    alv_am_win_set_focus(win_tools_calendar_item,tools_calendar_item_win_focus);
    alv_am_win_set_defocus(win_tools_calendar_item,tools_calendar_item_win_defocus);
    lv_obj_t *      win_tools_calendar_date = alv_am_win_create(app_tools,"tool_cal_date"); // 日历

    alv_am_win_set_active(win_tools_calendar_date,tools_calendar_date_value_win_active);
    alv_am_win_set_deactive(win_tools_calendar_date,tools_calendar_date_value_win_deactive);
    alv_am_win_set_focus(win_tools_calendar_date,tools_calendar_date_value_win_focus);
    alv_am_win_set_defocus(win_tools_calendar_date,tools_calendar_date_value_win_defocus);
    lv_obj_t *      win_tools_calendar_time = alv_am_win_create(app_tools,"tool_cal_time"); // 日历

    alv_am_win_set_active(win_tools_calendar_time,tools_calendar_time_value_win_active);
    alv_am_win_set_deactive(win_tools_calendar_time,tools_calendar_time_value_win_deactive);
    alv_am_win_set_focus(win_tools_calendar_time,tools_calendar_time_value_win_focus);
    alv_am_win_set_defocus(win_tools_calendar_time,tools_calendar_time_value_win_defocus);
    lv_obj_t *      win_tools_calendar_nongli = alv_am_win_create(app_tools,"tool_cal_nongli"); // 日历

    alv_am_win_set_active(win_tools_calendar_nongli,tools_calendar_nongli_win_active);
    alv_am_win_set_deactive(win_tools_calendar_nongli,tools_calendar_nongli_win_deactive);
    alv_am_win_set_focus(win_tools_calendar_nongli,tools_calendar_nongli_win_focus);
    alv_am_win_set_defocus(win_tools_calendar_nongli,tools_calendar_nongli_win_defocus);
    lv_obj_t *      win_tools_calendar_todo = alv_am_win_create(app_tools,"tool_cal_todo"); // 日历

    alv_am_win_set_active(win_tools_calendar_todo,tools_calendar_todo_win_active);
    alv_am_win_set_deactive(win_tools_calendar_todo,tools_calendar_todo_win_deactive);
    alv_am_win_set_focus(win_tools_calendar_todo,tools_calendar_todo_win_focus);
    alv_am_win_set_defocus(win_tools_calendar_todo,tools_calendar_todo_win_defocus);
    lv_obj_t *      win_tools_calendar_alarm = alv_am_win_create(app_tools,"tool_cal_alarm"); // 日历

    alv_am_win_set_active(win_tools_calendar_alarm,tools_calendar_alarm_win_active);
    alv_am_win_set_deactive(win_tools_calendar_alarm,tools_calendar_alarm_win_deactive);
    alv_am_win_set_focus(win_tools_calendar_alarm,tools_calendar_alarm_win_focus);
    alv_am_win_set_defocus(win_tools_calendar_alarm,tools_calendar_alarm_win_defocus);
    lv_obj_t *      win_tools_calendar_tododel = alv_am_win_create(app_tools,"tool_cal_tododel"); // 日历

    alv_am_win_set_active(win_tools_calendar_tododel,tools_calendar_tododel_win_active);
    alv_am_win_set_deactive(win_tools_calendar_tododel,tools_calendar_tododel_win_deactive);
    alv_am_win_set_focus(win_tools_calendar_tododel,tools_calendar_tododel_win_focus);
    alv_am_win_set_defocus(win_tools_calendar_tododel,tools_calendar_tododel_win_defocus);
    lv_obj_t *      win_tools_calendar_todobrowser = alv_am_win_create(app_tools,"tool_cal_browser"); // 日历

    alv_am_win_set_active(win_tools_calendar_todobrowser,tools_calendar_todobrowser_win_active);
    alv_am_win_set_deactive(win_tools_calendar_todobrowser,tools_calendar_todobrowser_win_deactive);
    alv_am_win_set_focus(win_tools_calendar_todobrowser,tools_calendar_todobrowser_win_focus);
    alv_am_win_set_defocus(win_tools_calendar_todobrowser,tools_calendar_todobrowser_win_defocus);
    lv_obj_t *      win_tools_calendar_tododelsure = alv_am_win_create(app_tools,"tool_cal_delsure"); // 日历

    alv_am_win_set_active(win_tools_calendar_tododelsure,tools_calendar_tododelsure_win_active);
    alv_am_win_set_deactive(win_tools_calendar_tododelsure,tools_calendar_tododelsure_win_deactive);
    alv_am_win_set_focus(win_tools_calendar_tododelsure,tools_calendar_tododelsure_win_focus);
    alv_am_win_set_defocus(win_tools_calendar_tododelsure,tools_calendar_tododelsure_win_defocus);
    lv_obj_t *      win_tools_calendar_tododoing = alv_am_win_create(app_tools,"tool_cal_doing"); // 日历

    alv_am_win_set_active(win_tools_calendar_tododoing,tools_calendar_tododoing_win_active);
    alv_am_win_set_deactive(win_tools_calendar_tododoing,tools_calendar_tododoing_win_deactive);
    alv_am_win_set_focus(win_tools_calendar_tododoing,tools_calendar_tododoing_win_focus);
    alv_am_win_set_defocus(win_tools_calendar_tododoing,tools_calendar_tododoing_win_defocus);

#ifdef CONFIG_AUDIO_TTS
    lv_obj_t *      win_tts_option = alv_am_win_create(app_tools,"tools_tts"); // 全语音王

    alv_am_win_set_active(win_tts_option,tts_option_fp_win_active);
    alv_am_win_set_deactive(win_tts_option,tts_option_fp_win_deactive);
    alv_am_win_set_focus(win_tts_option,tts_option_fp_win_focus);
    alv_am_win_set_defocus(win_tts_option,tts_option_fp_win_defocus);


    lv_obj_t *      win_tools_tts_on_off = alv_am_win_create(app_tools,"tools_tts_on_off"); // 全语音王

    alv_am_win_set_active(win_tools_tts_on_off,tts_option_on_off_fp_win_active);
    alv_am_win_set_deactive(win_tools_tts_on_off,tts_option_on_off_fp_win_deactive);
    alv_am_win_set_focus(win_tools_tts_on_off,tts_option_on_off_fp_win_focus);
    alv_am_win_set_defocus(win_tools_tts_on_off,tts_option_on_off_fp_win_defocus);


    lv_obj_t *      win_tools_tts_vol = alv_am_win_create(app_tools,"tools_tts_vol"); // 全语音王

    alv_am_win_set_active(win_tools_tts_vol,tts_option_vol_fp_win_active);
    alv_am_win_set_deactive(win_tools_tts_vol,tts_option_vol_fp_win_deactive);
    alv_am_win_set_focus(win_tools_tts_vol,tts_option_vol_fp_win_focus);
    alv_am_win_set_defocus(win_tools_tts_vol,tts_option_vol_fp_win_defocus);


    lv_obj_t *      win_tools_tts_time_auto = alv_am_win_create(app_tools,"tools_tts_time"); // 全语音王

    alv_am_win_set_active(win_tools_tts_time_auto,tts_option_time_auto_fp_win_active);
    alv_am_win_set_deactive(win_tools_tts_time_auto,tts_option_time_auto_fp_win_deactive);
    alv_am_win_set_focus(win_tools_tts_time_auto,tts_option_time_auto_fp_win_focus);
    alv_am_win_set_defocus(win_tools_tts_time_auto,tts_option_time_auto_fp_win_defocus);

    lv_obj_t *      win_tools_tts_time_set = alv_am_win_create(app_tools,"tools_tts_time_set"); // 全语音王

    alv_am_win_set_active(win_tools_tts_time_set,tts_option_time_set_fp_win_active);
    alv_am_win_set_deactive(win_tools_tts_time_set,tts_option_time_set_fp_win_deactive);
    alv_am_win_set_focus(win_tools_tts_time_set,tts_option_time_set_fp_win_focus);
    alv_am_win_set_defocus(win_tools_tts_time_set,tts_option_time_set_fp_win_defocus);

    lv_obj_t *      win_tools_tts_time_a = alv_am_win_create(app_tools,"tools_tts_time_a"); // 全语音王

    alv_am_win_set_active(win_tools_tts_time_a,tts_option_time_select_fp_win_active);
    alv_am_win_set_deactive(win_tools_tts_time_a,tts_option_time_select_fp_win_deactive);
    alv_am_win_set_focus(win_tools_tts_time_a,tts_option_time_select_fp_win_focus);
    alv_am_win_set_defocus(win_tools_tts_time_a,tts_option_time_select_fp_win_defocus);
#endif
}


