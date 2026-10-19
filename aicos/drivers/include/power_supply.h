/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines power supply driver api.
 *    The power supply api will be called by aicos user functions.
 */

#ifndef __POWER_SUPPLY_H__
#define __POWER_SUPPLY_H__
#include <bcd.h>

enum {
    POWER_SUPPLY_STATUS_UNKNOW = 0,
    POWER_SUPPLY_STATUS_CHARGING,
    POWER_SUPPLY_STATUS_DISCHARGING,/*usb plug out*/
    POWER_SUPPLY_STATUS_NOT_CHARGING,/*usb plugin,but system power consumption more than charge */
    POWER_SUPPLY_STATUS_FULL,
    POWER_SUPPLY_STATUS_MAX,
};

enum {
    PSY_INFO_CUR_VOLTAGE = 0,
    PSY_INFO_CAP,
    PSY_INFO_CHG_STATUS,
    PSY_INFO_CHG_TYPE,
    PSY_INFO_BAT_MAX_VOL,
    PSY_INFO_BAT_MIN_VOL,
};

struct psy_info {
    char psy_name[50];          /*power supply name*/
    u32 bat_current_voltage;    /*current ocv value, units in mV*/
    u32 bat_min_voltage;        /*battery min voltage, units in mV*/
    u32 bat_max_voltage;        /*battery max voltage, units in mV*/
    u8 bat_capacity;            /*current battery level*/
    u8 bat_chg_status;          /*current battery charging state*/
    u8 bat_chg_type;            /*charging type, dcp,cdp,sdp,no-dcd*/
};

struct psy_bat_mon_info{
    u8 bat_mon_enabled;         /*battery monitor state*/
    u32 bat_mon_cycle_ms;       /*battery monitor cycle ms*/
    u32 bat_mon_chging_cycle_ms;/*battery monitor cycle ms*/
};

typedef void (*psy_noticer_call)(const struct psy_info *psy_info);

/*power supply subsys internal interface.*/
int bat_cap_notice_threshold(void);
int psy_info_update(int info_type, int val);
void psy_has_changed(void);
bool psy_is_charging_complete(void);
bool psy_is_charging_enabled(void);
void psy_bat_monitor_cycle_set(u32 cycles_ms);
void psy_bat_mon_state_get(struct psy_bat_mon_info *p_bat_mon);

/**
 * __psy_charging_enable - charge switch control, internal interface.
 * @enabled: switch state.
 * non return.
 */
int __psy_charging_enable(bool enabled);

/**
 * psy_charging_enable - charge switch control, user interface.
 * @enabled: switch state.
 * non return.
 */
int psy_charging_enable(bool enabled);

int psy_clear_chg_complete_state(void);

void aic_battery_monitor_start(void);
void aic_battery_monitor_stop(void);

/**
 * psy_cc_current_set - cc current set.
 * @cc_current_mA: max cc current set, units in mA,.
 * return 0 if set success, else fail.
 */
int psy_cc_current_set(u32 cc_current_mA);

/**
 * psy_cc_current_get - cc current get.
 * @cc_current_mA: max cc current get, units in mA,.
 * return 0 if get success, else fail.
 */
int psy_cc_current_get(u32 *cc_current_mA);

/**
 * psy_rechg_ocv_set - recharger ocv voltage set.
 * @ocv_rechg_mv: recharger ocv voltate, units in mV,.
 * return 0 if set success, else fail.
 */
int psy_rechg_ocv_set(u32 ocv_rechg_mv);

/**
 * psy_rechg_ocv_get - recharger ocv voltage get.
 * @ocv_rechg_mv: recharger ocv voltate, units in mV,.
 * return 0 if get success, else fail.
 */
int psy_rechg_ocv_get(u32 *ocv_rechg_mv);

/**
 * psy_changed_notifier_register - register power supply status changed notifier callback.
 * @noticer_name: noticer name, user define.
 * @noticer_call: noticer callback, when power supply status changed will be notified.
 * return 0 if set success, else fail.
 */
int psy_changed_notifier_register(char *noticer_name, psy_noticer_call noticer_call);

/**
 * psy_changed_notifier_unregister - unregister power supply status changed notifier callback.
 * @noticer_name: noticer name, user define, alread register before this time.
 * return 0 if set success, else fail.
 */
int psy_changed_notifier_unregister(char *noticer_name);

/**
 * get_psy_info - get current power supply information.
 * @psy_info: current power supply total information.
 * non return.
 */
void get_psy_info(struct psy_info *psy_info);

/**
 * psy_get_bat_total_mah - psy get battery total mah.
 * return battery total mah.
 */
int psy_get_bat_total_mah(void);

/**
 * psy_adjust_monitor_timer - adjust battery charge monitor timer interval.
 * @interval_ms: units in mA, 0xFFFF_FFFF disable monitor timer.
 * non return.
 */
void psy_adjust_monitor_timer(u32 interval_ms);
#endif /* __POWER_SUPPLY_H__ */
