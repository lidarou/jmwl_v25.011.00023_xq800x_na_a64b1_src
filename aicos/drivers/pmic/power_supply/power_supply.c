/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is power supply core.
 *
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x086db4
#define LOG_MODULE    AMOD_PMIC
#include <aic_osa.h>
#include <aic_type.h>
#include <aic_boot_mode.h>
#include <aic_at_process_common.h>
#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <bcd.h>
#include <aic_list.h>
#include <power_supply.h>

static LIST_HEAD(psy_noticer_list);

#define PSY_HAS_CHANGED 0x1

aos_mutex_ptr psy_info_lock = AOS_NULL;
aos_mutex_ptr psy_list_lock = AOS_NULL;
aos_event_ptr psy_changed;

extern struct psy_bat_mon_info bat_mon_info;
struct psy_info sys_psy_info;

struct psy_noticer {
    char *name;
    struct list_head noticer_list;
    psy_noticer_call noticer_call;
};

static aos_status psy_lock(aos_mutex_ptr mutex)
{
    int ret = AOS_SUCCESS;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(ret == AOS_SUCCESS);
    }

    return ret;
}

static void psy_unlock(aos_mutex_ptr mutex)
{
    if (aos_is_user_thread_context()) {
        aos_put_mutex(mutex);
    }
}

void psy_has_changed(void)
{
    aos_set_event(psy_changed, PSY_HAS_CHANGED, AOS_OR);
}

int psy_changed_notifier_register(char *noticer_name,
                                          psy_noticer_call noticer_call)
{
    struct psy_noticer * psy_noticer_node;
    int ret = 0;

    if (!noticer_call) {
        aicx_warn(0x03e038, "s", "%s NULL noticer call!\n", __func__);
        ret = AIC_ENOENT;
        goto out;
    }

    psy_noticer_node = aos_zalloc(sizeof(struct psy_noticer));
    if (!psy_noticer_node) {
        aicx_warn(0x02ad96, "s", "%s not enough space!\n", __func__);
        ret = AIC_ENOMEM;
        goto out;
    }

    psy_lock(psy_list_lock);
    list_head_init(&psy_noticer_node->noticer_list);
    psy_noticer_node->name = strdup(noticer_name);
    psy_noticer_node->noticer_call = noticer_call;
    list_add_tail(&psy_noticer_node->noticer_list, &psy_noticer_list);
    psy_unlock(psy_list_lock);

out:
    return ret;
}

int psy_changed_notifier_unregister(char *noticer_name)
{
    struct psy_noticer *noticer_op, *noticer_temp;
    int ret = 0;

    psy_lock(psy_list_lock);
    list_for_each_entry_safe(noticer_op, noticer_temp, &psy_noticer_list,
                            noticer_list, struct psy_noticer) {
        if (!strcmp(noticer_op->name, noticer_name)) {
            list_del(&noticer_op->noticer_list);
            ret = aos_free(noticer_op->name);
            if (ret)
                aicx_warn(0x11807c, "s", "%s free user name fail!\n", __func__);


            ret |= aos_free(noticer_op);
            if (ret)
                aicx_warn(0x085bf7, "s", "%s free user fail!\n", __func__);

            break;
        }
    }
    psy_unlock(psy_list_lock);

    return ret;
}

void psy_notice_task(void *input)
{
    struct psy_noticer *noticer_temp = NULL;
    struct psy_info psy_info_temp;
    u32 psy_event;

    while(1) {
        aos_get_event(psy_changed, PSY_HAS_CHANGED, AOS_AND_CLEAR,
                     &psy_event, AOS_WAIT_FOREVER);

        memset(&psy_info_temp, 0, sizeof(psy_info_temp));
        psy_lock(psy_list_lock);
        memcpy(&psy_info_temp, &sys_psy_info, sizeof(psy_info_temp));

        list_for_each_entry(noticer_temp, &psy_noticer_list, noticer_list, struct psy_noticer) {
            noticer_temp->noticer_call(&psy_info_temp);
        }

        psy_unlock(psy_list_lock);
    }
}

void get_psy_info(struct psy_info *psy_info)
{
    psy_lock(psy_info_lock);
    memcpy(psy_info, &sys_psy_info, sizeof(struct psy_info));
    psy_unlock(psy_info_lock);
}

int psy_info_update(int info_type, int val)
{
    int ret = 0;

    psy_lock(psy_info_lock);
    switch (info_type) {
    case PSY_INFO_CUR_VOLTAGE:
        sys_psy_info.bat_current_voltage = val;
        break;
    case PSY_INFO_CAP:
        if (abs(val - sys_psy_info.bat_capacity) && !(val % bat_cap_notice_threshold())) {
            sys_psy_info.bat_capacity = val;
            psy_has_changed();
        } else {
            sys_psy_info.bat_capacity = val;
        }
        break;
    case PSY_INFO_CHG_STATUS:
        if (val != sys_psy_info.bat_chg_status) {
            sys_psy_info.bat_chg_status = val;
            psy_has_changed();
        }
        break;
    case PSY_INFO_CHG_TYPE:
        sys_psy_info.bat_chg_type = val;
        break;
    case PSY_INFO_BAT_MAX_VOL:
        sys_psy_info.bat_max_voltage = val;
        break;
    case PSY_INFO_BAT_MIN_VOL:
        sys_psy_info.bat_min_voltage = val;
        break;
    default:
        ret = AIC_EBADF;
        aicx_warn(0x0c5a5f, "", "us_p_up\n");
        break;
    }
    psy_unlock(psy_info_lock);

    return ret;
}

void psy_bcd_func(void *param)
{
    int bcd_type = (int)param;

    psy_lock(psy_info_lock);
    switch (bcd_type) {
    case AIC_BC_TYPE_NO_DCD:
        sys_psy_info.bat_chg_type = AIC_BC_TYPE_NO_DCD;
        break;
    case AIC_BC_TYPE_PS2:
        sys_psy_info.bat_chg_type = AIC_BC_TYPE_PS2;
        break;
    case AIC_BC_TYPE_SDP:
        sys_psy_info.bat_chg_type = AIC_BC_TYPE_SDP;
        break;
    case AIC_BC_TYPE_CDP:
        sys_psy_info.bat_chg_type = AIC_BC_TYPE_CDP;
        break;
    case AIC_BC_TYPE_DCP:
        sys_psy_info.bat_chg_type = AIC_BC_TYPE_DCP;
        break;
    case AIC_USB_UNPLUG:
        sys_psy_info.bat_chg_status = POWER_SUPPLY_STATUS_DISCHARGING;
        sys_psy_info.bat_chg_type = AIC_BC_TYPE_DUMMY;
        psy_adjust_monitor_timer(bat_mon_info.bat_mon_cycle_ms);
        goto out;
        break;
    default:
        sys_psy_info.bat_chg_status =POWER_SUPPLY_STATUS_UNKNOW;
        break;
    }

    if (aic_is_usb_plugin()) {
        psy_adjust_monitor_timer(bat_mon_info.bat_mon_chging_cycle_ms);
        if (psy_is_charging_complete()) {
            sys_psy_info.bat_chg_status = POWER_SUPPLY_STATUS_FULL;
            sys_psy_info.bat_capacity = 100;
        } else {
            sys_psy_info.bat_chg_status = POWER_SUPPLY_STATUS_CHARGING;
        }
    }

out:
    psy_has_changed();

    psy_unlock(psy_info_lock);
}

int psy_bcd_init(void)
{
    struct aic_bc_device *no_dcd, *unplug;
    int ret = 0;

    no_dcd = aos_zalloc(sizeof(struct aic_bc_device));
    no_dcd->bc_type = AIC_BC_TYPE_NO_DCD;
    strcpy(no_dcd->bcd_name, "psy_no_dcd");
    ret = aic_bcd_register(no_dcd, psy_bcd_func, (void *)AIC_BC_TYPE_NO_DCD);
    if(ret)
        aicx_warn(0x18aee5, "sd", "%s %d rf\n", __func__, no_dcd->bc_type);

#ifndef CONFIG_BCD_DUMMY
    struct aic_bc_device *ps2, *sdp, *cdp, *dcp;

    ps2 = aos_zalloc(sizeof(struct aic_bc_device));
    ps2->bc_type = AIC_BC_TYPE_PS2;
    strcpy(ps2->bcd_name, "psy_ps2");
    ret = aic_bcd_register(ps2, psy_bcd_func, (void *)AIC_BC_TYPE_PS2);
    if(ret)
        aicx_warn(0x149c29, "sd", "%s bcd %d register fail!\n", __func__, ps2->bc_type);

    sdp = aos_zalloc(sizeof(struct aic_bc_device));
    sdp->bc_type = AIC_BC_TYPE_SDP;
    strcpy(sdp->bcd_name, "psy_sdp");
    ret |= aic_bcd_register(sdp, psy_bcd_func, (void *)AIC_BC_TYPE_SDP);
    if(ret)
        aicx_warn(0x149c29, "sd", "%s bcd %d register fail!\n", __func__, sdp->bc_type);

    cdp = aos_zalloc(sizeof(struct aic_bc_device));
    cdp->bc_type = AIC_BC_TYPE_CDP;
    strcpy(cdp->bcd_name, "psy_cdp");
    ret |= aic_bcd_register(cdp, psy_bcd_func, (void *)AIC_BC_TYPE_CDP);
    if(ret)
        aicx_warn(0x149c29, "sd", "%s bcd %d register fail!\n", __func__, cdp->bc_type);

    dcp = aos_zalloc(sizeof(struct aic_bc_device));
    dcp->bc_type = AIC_BC_TYPE_DCP;
    strcpy(dcp->bcd_name, "psy_dcp");
    ret |= aic_bcd_register(dcp, psy_bcd_func, (void *)AIC_BC_TYPE_DCP);
    if(ret)
        aicx_warn(0x149c29, "sd", "%s bcd %d register fail!\n", __func__, dcp->bc_type);
#endif

    unplug = aos_zalloc(sizeof(struct aic_bc_device));
    unplug->bc_type = AIC_USB_UNPLUG;
    strcpy(unplug->bcd_name, "psy_unplug");
    ret |= aic_bcd_register(unplug, psy_bcd_func, (void *)AIC_USB_UNPLUG);
    if(ret)
        aicx_warn(0x149c29, "sd", "%s bcd %d register fail!\n", __func__, unplug->bc_type);

    return ret;
}

static void sys_power_supply_info_init(void)
{
    strcpy(sys_psy_info.psy_name, "spsy");/*system_power_supply*/
    sys_psy_info.bat_chg_type = AIC_USB_UNPLUG;
    sys_psy_info.bat_chg_status = POWER_SUPPLY_STATUS_DISCHARGING;
    sys_psy_info.bat_capacity = 0;
}

int power_supply_init(void)
{
    aos_thread_id psy_noticer_task;;
    int ret = 0;

    if (aic_boot_mode_is_gxem())
        return ret;

    psy_info_lock = aos_create_mutex("pinfo");/*psy_info_mutex*/
    AIC_ASSERT(psy_info_lock);

    psy_list_lock = aos_create_mutex("plist");/*psy_list_mutex*/
    AIC_ASSERT(psy_list_lock);

    psy_changed = aos_create_event("pchged");/*psy_changed_event*/
    AIC_ASSERT(psy_changed);

    sys_power_supply_info_init();

#ifndef CONFIG_USB_DETECT_GPIO
    ret = psy_bcd_init();
    if (ret)
        return ret;
#endif

    psy_noticer_task = aos_create_thread("pnoti", psy_notice_task, AOS_NULL, 1024, 0,
                      NULL, 16, AOS_AUTO_START);
    if (psy_noticer_task == AOS_INVALID_THREAD_ID)
        ret = AIC_ENODEV;

    return ret;
}
AIC_DRV_P1_INIT_EXPORT(power_supply_init);

#ifdef CONFIG_USB_DETECT_GPIO
#include <usb_service.h>

void psy_usb_plug_cb(u32 plug_state)
{
    psy_lock(psy_info_lock);
    switch (plug_state) {
    case USB_PLUG_DEFAULT_IN:
    case USB_PLUG_IN:
        sys_psy_info.bat_chg_type = AIC_BC_TYPE_NO_DCD;
        psy_adjust_monitor_timer(bat_mon_info.bat_mon_chging_cycle_ms);
        if (psy_is_charging_complete()) {
            sys_psy_info.bat_chg_status = POWER_SUPPLY_STATUS_FULL;
            sys_psy_info.bat_capacity = 100;
        } else {
            sys_psy_info.bat_chg_status = POWER_SUPPLY_STATUS_CHARGING;
        }
        break;
    case USB_PLUG_OUT:
    case USB_PLUG_DEFAULT_OUT:
        sys_psy_info.bat_chg_status = POWER_SUPPLY_STATUS_DISCHARGING;
        sys_psy_info.bat_chg_type = AIC_BC_TYPE_DUMMY;
        psy_adjust_monitor_timer(bat_mon_info.bat_mon_cycle_ms);
        goto out;
        break;
    default:
        sys_psy_info.bat_chg_status =POWER_SUPPLY_STATUS_UNKNOW;
        break;
    }

out:
    psy_has_changed();

    psy_unlock(psy_info_lock);
}

int psy_gpio_usb_init(void)
{
    int ret =0;

    ret = usb_service_plug_event_listen_register(psy_usb_plug_cb);

    return ret;
}
DIS_BASIC_COMP_P2_INIT_EXPORT(psy_gpio_usb_init);
#endif
