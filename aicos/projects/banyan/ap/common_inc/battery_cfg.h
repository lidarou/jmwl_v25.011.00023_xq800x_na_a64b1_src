/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/
#ifndef __BATTERY_CFG_H__
#define __BATTERY_CFG_H__
struct psy_ocv_table {
    int ocv_voltage; /*mv*/
    int capacity;   /*percent*/
};

#define     BAT_MONITOR_CYCLE_MS        20000
#define     BAT_MONITOR_CYCLE_CHGING_MS 5000
#define     VBAT_SHUTDOWN_VAL_MV        3300
#define     VBAT_RECORD_DPETH           10
#define     VBAT_DROP_TOLERANCE         100 //mv
#define     BAT_CAP_NOTICE_THRESHOLD    5 //5% notice psy has changed
#define     RECHG_OCV_MV                4000//mv
#define     BAT_TOTAL_MHA               800//mAH
struct psy_ocv_table ocv_table[] = {
    {4150, 100}, {4022, 90}, {3988, 80},
    {3940,70}, {3882,60}, {3842,50},
    {3806,40}, {3758,30}, {3664,20},
    {3504,10}, {3392,0}
};

#define     VBAT_MAX_VOLTAGE        4200
#define     VBAT_MIN_VOLTAGE        3400
#define     CHARGE_MAX_CURRENT      600

#define     BAT_INTERNAL_RESISTANCE_MOHMS       200
#define     POWER_SUPPLY_CC_CURRENT             500
#define     MIN_CC_CV_VOLTAGE                   4150
#endif /*__BATTERY_CFG_H__*/
