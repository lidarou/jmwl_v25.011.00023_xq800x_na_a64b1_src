/*****************************************************************************/
/*                                                                           */
/*   Copyright 20233 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the psy demo for user
*/

#include <aic_boot_mode.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aic_type.h>
#include <aic_osa_ex.h>
#include <power_supply.h>


#define PSY_SAMPLE_THRD_PRI         16
#define PSY_SAMPLE_STACK_SIZE       SIZE_1K
#define PSY_SAMPLE_QUEUE_NUM        8

struct psy_info_print {
    int type;
    char *name;
};

struct psy_info_print type_info[AIC_BC_TYPE_MAX] = {
    {AIC_BC_TYPE_NO_DCD, "no_dcd"},
    {AIC_BC_TYPE_PS2,    "ps2"},
    {AIC_BC_TYPE_SDP,    "sdp"},
    {AIC_BC_TYPE_CDP,    "cdp"},
    {AIC_BC_TYPE_DCP,    "dcp"},
    {AIC_USB_UNPLUG,     "usb_unplug"},
    {AIC_BC_TYPE_DUMMY,  "dummy"},
};

struct psy_info_print status_info[POWER_SUPPLY_STATUS_MAX] = {
    {POWER_SUPPLY_STATUS_UNKNOW,        "STATUS_UNKNOW"},
    {POWER_SUPPLY_STATUS_CHARGING,      "STATUS_CHARGING"},
    {POWER_SUPPLY_STATUS_DISCHARGING,   "STATUS_DISCHARGING"},
    {POWER_SUPPLY_STATUS_NOT_CHARGING,  "STATUS_NOT_CHARGING"},
    {POWER_SUPPLY_STATUS_FULL,          "STATUS_FULL"},
};

void psd_demo_call1(const struct psy_info *psy_info)
{

    drv_logw("%s psy_name:%s bat_vol:%d bat_cap:%d chg_status%s "
             "chg_type:%s bat_min_voltage:%d bat_max_voltage:%d\n",
             __func__,
             psy_info->psy_name,
             psy_info->bat_current_voltage,
             psy_info->bat_capacity,
             status_info[psy_info->bat_chg_status].name,
             type_info[psy_info->bat_chg_type].name,
             psy_info->bat_min_voltage,
             psy_info->bat_max_voltage);
}

void psd_demo_call2(const struct psy_info *psy_info)
{

    drv_logw("%s psy_name:%s bat_vol:%d bat_cap:%d chg_status%s "
             "chg_type:%s bat_min_voltage:%d bat_max_voltage:%d\n",
             __func__,
             psy_info->psy_name,
             psy_info->bat_current_voltage,
             psy_info->bat_capacity,
             status_info[psy_info->bat_chg_status].name,
             type_info[psy_info->bat_chg_type].name,
             psy_info->bat_min_voltage,
             psy_info->bat_max_voltage);
}

void psy_demo_task(void *input)
{
    struct psy_info psy_info;

    while(1) {
        get_psy_info(&psy_info);

        drv_logw("%s psy_name:%s bat_vol:%d bat_cap:%d chg_status%s "
                 "chg_type:%s bat_min_voltage:%d bat_max_voltage:%d\n",
                 __func__,
                 psy_info.psy_name,
                 psy_info.bat_current_voltage,
                 psy_info.bat_capacity,
                 status_info[psy_info.bat_chg_status].name,
                 type_info[psy_info.bat_chg_type].name,
                 psy_info.bat_min_voltage,
                 psy_info.bat_max_voltage);
        aos_sleep(5000);
    }
}

int psd_demo(void)
{
    int ret;

    if (aic_boot_mode_is_gxem()) {
        drv_logw("charger already disable before kernel init, don't start psd!\n", __func__);
        return ret;
    }

    /*power supply status notification.*/
    ret = psy_changed_notifier_register("psy_demo1", psd_demo_call1);
    if (ret)
        drv_logw("psy demo1 init fail!!\n");

    ret = psy_changed_notifier_register("psy_demo2", psd_demo_call2);
    if (ret)
        drv_logw("psy demo2 init fail!!\n");

    ret = psy_changed_notifier_register("psy_demo2", psd_demo_call2);
    if (ret)
        drv_logw("psy demo2 init fail!!\n");

    /*power supply status monitor task*/
    aos_create_thread("psy_demo_task",
                      psy_demo_task,
                      AOS_NULL,
                      PSY_SAMPLE_STACK_SIZE,
                      PSY_SAMPLE_QUEUE_NUM,
                      "psy_demo_task",
                      PSY_SAMPLE_THRD_PRI,
                      AOS_AUTO_START);

    return 0;
}
AIC_NORMAL_APP_L1_INIT_EXPORT(psd_demo);
