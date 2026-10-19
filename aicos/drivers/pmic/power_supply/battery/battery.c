/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of battery module
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0c13fc
#define LOG_MODULE    AMOD_PMIC

#include <aic_init.h>
#include <aic_errno.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <aic_osa.h>
#include <adc.h>
#include <battery_cfg.h>
#include <irq_chip.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include <pmic_irq.h>
#include <slp_inc.h>
#include <sysoff.h>
#include <power_supply.h>
#include <aic_boot_mode.h>
#include <pmu_user.h>

#define     BAT_MONITOR_TRIG            0x1
#define     LOW_BAT_SHUTDOWN_CNT        0x3

aos_timer_ptr   battery_monitor_timer;
aos_event_ptr   battery_monitor_event;
aos_mutex_ptr   battery_lock = AOS_NULL;

static int low_bat_threshold_cnt = 0;
static u32 rechg_ocv_mv = RECHG_OCV_MV;
u32 vbat_record_list[VBAT_RECORD_DPETH] = {0};
static int vbat_cur = 0;
int vbat_record_idx = 0;

struct psy_bat_mon_info bat_mon_info = {0, BAT_MONITOR_CYCLE_MS, BAT_MONITOR_CYCLE_CHGING_MS};

static aos_status bat_lock(void)
{
    int ret = AOS_SUCCESS;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(battery_lock, AOS_WAIT_FOREVER);
        AIC_ASSERT(ret == AOS_SUCCESS);
    }

    return ret;
}

static void bat_unlock(void)
{
    if (aos_is_user_thread_context()) {
        aos_put_mutex(battery_lock);
    }
}

static int psy_ocv2cap(struct psy_ocv_table *ocv_tab,
                          int tab_len, int ocv)
{
    int i, cap, temp;

    for (i = 0; i < tab_len; i++)
        if (ocv > ocv_tab[i].ocv_voltage)
            break;

    if (i > 0 && i < tab_len) {
        temp = (ocv_tab[i - 1].capacity - ocv_tab[i].capacity) *
            (ocv - ocv_tab[i].ocv_voltage);
        temp /= ocv_tab[i - 1].ocv_voltage - ocv_tab[i].ocv_voltage;
        cap = temp + ocv_tab[i].capacity;
    } else if (i == 0) {
        cap = ocv_tab[0].capacity;
    } else {
        cap = ocv_tab[tab_len - 1].capacity;
    }

    return cap;
}

static void vbat_avg_init(void);
void aic_battery_monitor_start(void)
{
    aos_change_timer(battery_monitor_timer, bat_mon_info.bat_mon_cycle_ms, AOS_BE_PERIOD);
    bat_mon_info.bat_mon_enabled = 1;
    bat_lock();
    vbat_avg_init();
    bat_unlock();
}

void aic_battery_monitor_stop(void)
{
    bat_mon_info.bat_mon_enabled = 0;
    aos_deactivate_timer(battery_monitor_timer);
}

void psy_adjust_monitor_timer(u32 interval_ms)
{
    if(!battery_monitor_timer || !bat_mon_info.bat_mon_enabled)
        return;

    if (interval_ms == 0xFFFFFFFF) {
        bat_mon_info.bat_mon_enabled = 0;
        aos_deactivate_timer(battery_monitor_timer);
    } else {
        aos_change_timer(battery_monitor_timer, interval_ms, AOS_BE_PERIOD);
        bat_mon_info.bat_mon_enabled = 1;
    }
}

void psy_bat_monitor_cycle_set(u32 cycles_ms)
{
    bat_mon_info.bat_mon_cycle_ms = cycles_ms;
}

void psy_bat_mon_state_get(struct psy_bat_mon_info *p_bat_mon)
{
    memcpy(p_bat_mon, &bat_mon_info, sizeof(struct psy_bat_mon_info));
}

int psy_charging_enable(bool enabled)
{
    int ret;

    bat_lock();
    ret = __psy_charging_enable(enabled);
    bat_unlock();

    return ret;
}

static void vbat_cur_update(void)
{
    struct psy_info cur_psy_info;
    int ocv_table_len;
    u32 temp = 0;
    int cap;
    int i;

    for (i = 0; i < VBAT_RECORD_DPETH; i ++) {
        temp += vbat_record_list[i];
    }

    vbat_cur = temp / VBAT_RECORD_DPETH;

    ocv_table_len = sizeof(ocv_table)/sizeof(ocv_table[0]);
    cap = psy_ocv2cap(ocv_table, ocv_table_len, vbat_cur);

    psy_info_update(PSY_INFO_CUR_VOLTAGE, vbat_cur);
    get_psy_info(&cur_psy_info);
    if ((cap > cur_psy_info.bat_capacity) && aic_is_usb_plugin())
        cap = cur_psy_info.bat_capacity + 1;
    else if ((cap < cur_psy_info.bat_capacity) && !aic_is_usb_plugin())
        cap = cur_psy_info.bat_capacity - 1;
    else
        cap = cur_psy_info.bat_capacity;

    if (aic_is_usb_plugin()) {
        if (vbat_cur < rechg_ocv_mv)
            psy_clear_chg_complete_state();

        if (psy_is_charging_complete() || (cap == 100)) {
            psy_info_update(PSY_INFO_CAP, 100);
            psy_info_update(PSY_INFO_CHG_STATUS, POWER_SUPPLY_STATUS_FULL);
        } else {
            psy_info_update(PSY_INFO_CAP, cap);
            psy_info_update(PSY_INFO_CHG_STATUS, POWER_SUPPLY_STATUS_CHARGING);
        }
    } else {
        psy_info_update(PSY_INFO_CAP, cap);
        psy_info_update(PSY_INFO_CHG_STATUS, POWER_SUPPLY_STATUS_DISCHARGING);
    }
}

static bool is_vbat_debounce(int vol)
{
    struct psy_info psy_info_cur;
    bool ret = false;

    get_psy_info(&psy_info_cur);

    if ((vol > vbat_cur) && (psy_info_cur.bat_chg_status == POWER_SUPPLY_STATUS_DISCHARGING))
        ret = true;
    else if ((vol < vbat_cur) && (psy_info_cur.bat_chg_status == POWER_SUPPLY_STATUS_CHARGING))
        ret = true;

    return ret;
}

static void vbat_record(int vol)
{
    bat_lock();

    if (is_vbat_debounce(vol))
        goto out;

    if ((vol - vbat_cur) > VBAT_DROP_TOLERANCE)
        vol = vbat_cur + VBAT_DROP_TOLERANCE / 2;
    else if  ((vol - vbat_cur) < -VBAT_DROP_TOLERANCE)
        vol = vbat_cur - VBAT_DROP_TOLERANCE / 2;

    vbat_record_list[vbat_record_idx++ % VBAT_RECORD_DPETH] = vol;
    if (vbat_record_idx == VBAT_RECORD_DPETH)
        vbat_record_idx = 0;

out:
    vbat_cur_update();
    bat_unlock();
}

static u32 get_vbat_vol(void)
{
    u32 vbat;

    bat_lock();

    if (aic_is_usb_plugin() && psy_is_charging_enabled()) {
        __psy_charging_enable(false);
        aos_sleep(10);
        vbat = vbat_vol_mv_get();
        __psy_charging_enable(true);
    } else {
        vbat = vbat_vol_mv_get();
    }

    bat_unlock();

    return vbat;
}

u32 my_get_vbat_vol(void)
{
    adc_self_cal();

    return get_vbat_vol();
}

void psy_reset_chg_state(void)
{
    bat_lock();

    __psy_charging_enable(false);
    aos_usleep(1000);
    __psy_charging_enable(true);

    bat_unlock();
}

int bat_cap_notice_threshold(void)
{
    return BAT_CAP_NOTICE_THRESHOLD;
}

void vbat_val_judgment(void)
{
    u32 vbat;

    vbat = get_vbat_vol();

    aicx_warn(0x1a9449, "d", "cur vbat:%d mv.\n", vbat);

    if (!aic_is_usb_plugin()) {
        if (vbat < VBAT_SHUTDOWN_VAL_MV) {
            aicx_error(0x140d86, "d", "vbat:%d mv too low, shutdown!!!\n", vbat);
            if (low_bat_threshold_cnt > LOW_BAT_SHUTDOWN_CNT)
                sysoff_shutdown();

            low_bat_threshold_cnt++;
        } else {
            low_bat_threshold_cnt = 0;
            aicx_debug(0x08e150, "d", "cur vbat value:%d mv\n", vbat);
        }
    }
#ifdef CONFIG_CHGR_AUTO_IC
    else {
        if ((vbat < MIN_CC_CV_VOLTAGE) && psy_is_charging_enabled())
            vbat = vbat - (BAT_INTERNAL_RESISTANCE_MOHMS * POWER_SUPPLY_CC_CURRENT) / 1000;
    }
#endif

    vbat_record(vbat);
}

u32 get_cur_avg_vol(void)
{
    u32 vbat_cur_temp;

    bat_lock();
    vbat_cur_temp = vbat_cur;
    bat_unlock();

    return vbat_cur_temp;
}

static void vbat_avg_init(void)
{
    int i, cap;

    vbat_cur = 0;
    for (i = 0; i < VBAT_RECORD_DPETH; i ++) {
        vbat_record_list[i] = get_vbat_vol();
        vbat_cur += vbat_record_list[i];
    }

    vbat_cur /= VBAT_RECORD_DPETH;
    vbat_record_idx = 0;

    cap = psy_ocv2cap(ocv_table, sizeof(ocv_table)/sizeof(ocv_table[0]), vbat_cur);

    psy_info_update(PSY_INFO_CUR_VOLTAGE, vbat_cur);
    psy_info_update(PSY_INFO_CAP, cap);
}

void bat_timer_func(void *param)
{
    aos_set_event(battery_monitor_event, BAT_MONITOR_TRIG, AOS_OR);
}

void battery_monitor_task(void *input)
{
    u32 e_bat_monitor;
    int ret;

    while(1) {
        ret = aos_get_event(battery_monitor_event, BAT_MONITOR_TRIG,
                            AOS_AND_CLEAR, &e_bat_monitor, AOS_WAIT_FOREVER);
        if (ret)
            continue;

        vbat_val_judgment();
    }
}

static void vbat_info_cfg(void)
{
    psy_info_update(PSY_INFO_BAT_MAX_VOL, VBAT_MAX_VOLTAGE);
    psy_info_update(PSY_INFO_BAT_MIN_VOL, VBAT_MIN_VOLTAGE);
}

int psy_rechg_ocv_set(u32 ocv_rechg_mv)
{
    int ret = AIC_SUCCESS;

    if (ocv_rechg_mv >= ocv_table[0].ocv_voltage)
        return AIC_EINVAL;

    rechg_ocv_mv = ocv_rechg_mv;

    return ret;
}

int psy_get_bat_total_mah(void)
{
    return BAT_TOTAL_MHA;
}

int psy_rechg_ocv_get(u32 *ocv_rechg_mv)
{
    *ocv_rechg_mv = rechg_ocv_mv;

    return AIC_SUCCESS;
}

int aic_battery_init(void)
{
    int ret = 0;

    battery_lock = aos_create_mutex("bat");
    AIC_ASSERT(battery_lock);

    if (aic_boot_mode_is_gxem()) {
        ret = psy_charging_enable(false);
        return ret;
    }

    vbat_info_cfg();

    vbat_avg_init();

    battery_monitor_event = aos_create_event("batm");/*battery_monitor_event*/
    AIC_ASSERT(battery_monitor_event);

    aos_create_thread("batmt", battery_monitor_task, AOS_NULL, 1024, 0,
                      NULL, 16, AOS_AUTO_START);/*battery_monitor_task*/

    battery_monitor_timer = aos_create_timer("bat_mti",/*battery_monitor_timer*/
                                         bat_timer_func, NULL, BAT_MONITOR_CYCLE_MS,
                                         AOS_AUTO_ACTIVATE, AOS_BE_PERIOD);
    bat_mon_info.bat_mon_cycle_ms = BAT_MONITOR_CYCLE_MS;

    if(!battery_monitor_timer) {
        aicx_error(0x09f5b3, "s", "%s f\n", __func__);
        ret = AIC_ENXIO;
        goto out;
    }

    aic_battery_monitor_start();

    if (aic_is_usb_plugin())
        psy_adjust_monitor_timer(bat_mon_info.bat_mon_chging_cycle_ms);

#ifdef POWER_SUPPLY_CC_CURRENT
    ret = psy_cc_current_set(POWER_SUPPLY_CC_CURRENT);
#endif

out:
    return ret;
}
AIC_COMP_P2_INIT_EXPORT(aic_battery_init);
