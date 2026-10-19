/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  battery charging device node operate handle file.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0dd32d
#define LOG_MODULE    AMOD_PMIC
#include <aic_boot_mode.h>
#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aic_type.h>
#include <aic_osa.h>
#include <adc.h>
#include <pmic_irq.h>
#include <pmu_user.h>
#include <bcd.h>
#include "bcd_core.h"
#include <sysoff.h>
#include <power_supply.h>
#include <tx_user.h>

struct bcd_status {
    int sys_usb_plug_in;
    u32 lpi_timestamp;  /*last plug in timestamp*/
    u32 lpo_timestamp;  /*last plug on timestamp*/
} bcd_recorder = {0, 0, 0};

struct bcd_list bcd_list_gropu[AIC_BC_TYPE_MAX] = {
    {AIC_BC_TYPE_NO_DCD, LIST_HEAD_INIT(bcd_list_gropu[AIC_BC_TYPE_NO_DCD].bcd_list_head)},
    {AIC_BC_TYPE_PS2, LIST_HEAD_INIT(bcd_list_gropu[AIC_BC_TYPE_PS2].bcd_list_head)},
    {AIC_BC_TYPE_SDP, LIST_HEAD_INIT(bcd_list_gropu[AIC_BC_TYPE_SDP].bcd_list_head)},
    {AIC_BC_TYPE_CDP, LIST_HEAD_INIT(bcd_list_gropu[AIC_BC_TYPE_CDP].bcd_list_head)},
    {AIC_BC_TYPE_DCP, LIST_HEAD_INIT(bcd_list_gropu[AIC_BC_TYPE_DCP].bcd_list_head)},
    {AIC_USB_UNPLUG, LIST_HEAD_INIT(bcd_list_gropu[AIC_USB_UNPLUG].bcd_list_head)}
};

struct bc_type_info bc_info[AIC_BC_TYPE_MAX] = {
        {AIC_BC_TYPE_NO_DCD, "no_dcd"},
        {AIC_BC_TYPE_PS2,    "ps2"},
        {AIC_BC_TYPE_SDP,    "sdp"},
        {AIC_BC_TYPE_CDP,    "cdp"},
        {AIC_BC_TYPE_DCP,    "dcp"},
        {AIC_USB_UNPLUG,     "usb_unplug"},
        {AIC_BC_TYPE_DUMMY,  "dummy"},
};

int aic_bcd_unregister(struct aic_bc_device *bc_dev)
{
    int ret = 0;
    struct bcd_dev *bcd_dev_ops, *temp_ops;

    if (bc_dev->bc_type >= AIC_BC_TYPE_MAX) {
        ret = AIC_ENOTSUP;
        aicx_warn(0x0447f8, "sd", "%s non-supported bc type %d!\n", __func__, bc_dev->bc_type);
        goto out;
    }

    list_for_each_entry_safe(bcd_dev_ops, temp_ops, &bcd_list_gropu[bc_dev->bc_type].bcd_list_head,
                            link_list, struct bcd_dev) {
        if (!strcmp(bcd_dev_ops->bcd_name, bc_dev->bcd_name)) {
            list_del(&bcd_dev_ops->link_list);
            ret = aos_free(bcd_dev_ops);
            if (ret)
                aicx_warn(0x0be34b, "s", "%s free node fail!\n", __func__);
            break;
        }
    }

out:
    return ret;
}

int aic_bcd_register(struct aic_bc_device *bc_dev, PMIC_BCD_FUNC bcd_f, void *uparam)
{
    int ret = 0;
    struct bcd_dev *new_bcd_dev;

    if (bc_dev->bc_type >= AIC_BC_TYPE_MAX) {
        ret = AIC_ENOTSUP;
        aicx_warn(0x0447f8, "sd", "%s non-supported bc type %d!\n", __func__, bc_dev->bc_type);
        goto out;
    }

    new_bcd_dev = aos_zalloc(sizeof(struct bcd_dev));
    if (!new_bcd_dev) {
        ret = AIC_ENOSPC;
        aicx_warn(0x02ad96, "s", "%s not enough space!\n", __func__);
        goto out;
    }

    list_head_init(&new_bcd_dev->link_list);

    new_bcd_dev->bcd_f = bcd_f;
    strcpy(new_bcd_dev->bc_type, bc_info[bc_dev->bc_type].type_name);
    strcpy(new_bcd_dev->bcd_name, bc_dev->bcd_name);
    new_bcd_dev->para = uparam;

    list_add(&new_bcd_dev->link_list, &bcd_list_gropu[bc_dev->bc_type].bcd_list_head);

out:
    return ret;
}

#ifdef CONFIG_USB_DETECT_GPIO
void bcd_handler(void *param)
{
    struct bcd_dev *bcd_dev_ops;
    int i;

    bcd_recorder.lpi_timestamp = TX_TRACE_TIME_SOURCE;
    bcd_recorder.sys_usb_plug_in = TRUE;

    for (i = 0; i <= AIC_BC_TYPE_DCP; i ++ ) {
        list_for_each_entry(bcd_dev_ops, &bcd_list_gropu[i].bcd_list_head,
                            link_list, struct bcd_dev) {
            bcd_dev_ops->bcd_f(bcd_dev_ops->para);
        }
    }
}

void usb_unplug_handler(void *param)
{
    struct bcd_dev *bcd_dev_ops;

    bcd_recorder.lpo_timestamp = TX_TRACE_TIME_SOURCE;
    bcd_recorder.sys_usb_plug_in = FALSE;

    list_for_each_entry(bcd_dev_ops, &bcd_list_gropu[AIC_USB_UNPLUG].bcd_list_head,
                       link_list, struct bcd_dev) {
        bcd_dev_ops->bcd_f(bcd_dev_ops->para);
    }
}

int aic_is_usb_plugin(void)
{
    return bcd_recorder.sys_usb_plug_in;
}

#else

static struct pmu_user *vddusb;
#ifdef CONFIG_COMPARE_VBUS_PLUG
#define         USB_PLUG_OUT_TRIG            0x1
aos_timer_ptr   plug_det_timer;
aos_event_ptr   usb_det_trig;
aos_thread_id   plug_out_broadcast;
extern struct pmu_user *pchg;
#else
#define         USB_ANTI_SHAKE_TRIG          0x1
aos_timer_ptr   usb_anti_shake_delay_timer;
aos_event_ptr   usb_anti_shake_trig;
aos_thread_id   usb_anti_shake_task_id;
#endif

void bcd_handler(void *param)
{
    struct bcd_dev *bcd_dev_ops;
    int i, bc_type, ret;

    bcd_recorder.lpi_timestamp = TX_TRACE_TIME_SOURCE;
    bcd_recorder.sys_usb_plug_in = TRUE;

    ret = pmu_enable(vddusb);
    if (ret) {
        aicx_warn(0x06e33d, "s", "%s enable fail!\n", __func__);
        return;
    }

    bc_type = aic_bcd_result();

    aicx_info(0x0af885, "ss", "%s bc_type:%s\n", __func__, bc_info[bc_type].type_name);

    if (bc_type == AIC_BC_TYPE_DUMMY)
        for (i = 0; i <= AIC_BC_TYPE_DCP; i ++ ) {
            list_for_each_entry(bcd_dev_ops, &bcd_list_gropu[i].bcd_list_head,
                                link_list, struct bcd_dev) {
                bcd_dev_ops->bcd_f(bcd_dev_ops->para);
            }
        }
    else
        list_for_each_entry(bcd_dev_ops, &bcd_list_gropu[bc_type].bcd_list_head,
                            link_list, struct bcd_dev) {
            bcd_dev_ops->bcd_f(bcd_dev_ops->para);
        }

    ret = pmu_disable(vddusb);
    if (ret)
        aicx_warn(0x036995, "s", "%s disable fail!\n", __func__);

#ifdef CONFIG_COMPARE_VBUS_PLUG
    aos_change_timer(plug_det_timer, USB_PLUG_OUT_MONITOR_CYCLE_MS, AOS_BE_PERIOD);
#else
    aos_change_timer(usb_anti_shake_delay_timer, USB_ANTI_SHAKE_TIME_MS, AOS_NO_PERIOD);
#endif
}

void usb_unplug_handler(void *param)
{
    struct bcd_dev *bcd_dev_ops;

    bcd_recorder.lpo_timestamp = TX_TRACE_TIME_SOURCE;
    bcd_recorder.sys_usb_plug_in = FALSE;

    list_for_each_entry(bcd_dev_ops, &bcd_list_gropu[AIC_USB_UNPLUG].bcd_list_head,
                       link_list, struct bcd_dev) {
        bcd_dev_ops->bcd_f(bcd_dev_ops->para);
    }

#ifndef CONFIG_COMPARE_VBUS_PLUG
    aos_change_timer(usb_anti_shake_delay_timer, USB_ANTI_SHAKE_TIME_MS, AOS_NO_PERIOD);
#endif
}

int aic_is_usb_plugin(void)
{
    return aic_chg_int_rise();
}

#ifdef CONFIG_COMPARE_VBUS_PLUG
extern void psy_reset_chg_state(void);
void plug_out_broadcast_task(void *input)
{
    u32 usb_monitor;
    int ret, i;

    while(1) {
        ret = aos_get_event(usb_det_trig, USB_PLUG_OUT_TRIG,
                            AOS_AND_CLEAR, &usb_monitor, AOS_WAIT_FOREVER);
        if (ret)
            continue;

        if (vbus_vol_mv_get() < VBUS_VOL_THOLD_MV) {
             i = 0;
            aos_deactivate_timer(plug_det_timer);

            if (pmu_is_enabled(pchg)) {
                psy_reset_chg_state();
            }

            while(aic_is_usb_plugin() && i < VBUS_STATUS_FINISH_WAIT_CNT) {
               aos_sleep(10);
               i++;
            }

            if (i == VBUS_STATUS_FINISH_WAIT_CNT) {
                aos_change_timer(plug_det_timer, USB_PLUG_OUT_MONITOR_CYCLE_MS, AOS_BE_PERIOD);
                continue;
            }

            usb_unplug_handler(NULL);
        }
    }
}

void usb_plug_detect_trig(void *param)
{
    aos_set_event(usb_det_trig, USB_PLUG_OUT_TRIG, AOS_OR);
}
#else

void usb_anti_shake_task(void *input)
{
    u32 bcd_monitor;
    int ret;

    while(1) {
        ret = aos_get_event(usb_anti_shake_trig, USB_ANTI_SHAKE_TRIG,
                            AOS_AND_CLEAR, &bcd_monitor, AOS_WAIT_FOREVER);
        if (ret)
            continue;

        if (aic_is_usb_plugin() && !bcd_recorder.sys_usb_plug_in) {
            aicx_warn(0x0544ae, "s", "%s pmu int shake, trigger anti shake!!! plug-in", __func__);
            bcd_handler(NULL);
        } else if ( !aic_is_usb_plugin() && bcd_recorder.sys_usb_plug_in) {
            aicx_warn(0x12c754, "s", "%s pmu int shake, trigger anti shake!!! plut-out", __func__);
            usb_unplug_handler(NULL);
        }
    }
}

void usb_anti_timer_action(void *param)
{
    aos_set_event(usb_anti_shake_trig, USB_ANTI_SHAKE_TRIG, AOS_OR);
}
#endif
int bcd_init(void)
{
    int ret;

    vddusb = pmu_get("bcd_detect", "VDDUSB3V1");
    if (!vddusb) {
        ret = AIC_ENXIO;
        aicx_warn(0x09b1f3, "s", "%s get pmu operate handler fail!\n", __func__);
        goto out;
    }

    /*register chg in irq, trigger bc1.2 detect process.*/
    ret = register_pmic_irq(PMIC_PMU_RTC_CHG_RISE, bcd_handler, NULL);
    if (ret) {
        aicx_warn(0x15c0be, "s", "%s chg rise handler register failed!!\n", __func__);
        goto out;
    }

#ifdef CONFIG_COMPARE_VBUS_PLUG
    usb_det_trig = aos_create_event("usb_det_trig");
    AIC_ASSERT(usb_det_trig);

    plug_out_broadcast = aos_create_thread("plug_out_broadcast", plug_out_broadcast_task, AOS_NULL, 1024, 8,
                      "plug_out_broadcast", USB_PLUG_OUT_PRIO, AOS_AUTO_START);
    if (plug_out_broadcast == AOS_INVALID_THREAD_ID) {
        aicx_error(0x095ce1, "sds", "%s(%d) %s fail\n", __FILE__, __LINE__, __func__);
        ret = AIC_EIO;
        goto out;
    }

    plug_det_timer = aos_create_timer("plug_det_timer",
                                         usb_plug_detect_trig, NULL, USB_PLUG_OUT_MONITOR_CYCLE_MS,
                                         AOS_NO_ACTIVATE, AOS_BE_PERIOD);
#else
        ret = register_pmic_irq(PMIC_PMU_RTC_CHG_FALL, usb_unplug_handler, NULL);
        if (ret) {
            aicx_warn(0x117c55, "s", "%s chg fall handler register failed!!\n", __func__);
            goto out;
        }

    usb_anti_shake_trig = aos_create_event("usb_anti_shake_trig");
    AIC_ASSERT(usb_anti_shake_trig);

    usb_anti_shake_task_id = aos_create_thread("usb_anti_shake_task", usb_anti_shake_task, AOS_NULL, 1024, 0,
                      NULL, USB_PLUG_OUT_PRIO, AOS_AUTO_START);
    if (usb_anti_shake_task_id == AOS_INVALID_THREAD_ID) {
        aicx_error(0x095ce1, "sds", "%s(%d) %s fail\n", __FILE__, __LINE__, __func__);
        ret = AIC_EIO;
        goto out;
    }

    usb_anti_shake_delay_timer = aos_create_timer("usb_anti_shake_delay_timer",
                                         usb_anti_timer_action, NULL, USB_ANTI_SHAKE_TIME_MS,
                                         AOS_NO_ACTIVATE, AOS_NO_PERIOD);
#endif
    /*workaround for charger in power-on status non-charger-in irq*/
    if (aic_chg_int_rise())
        bcd_handler(NULL);
    else
        if (aic_boot_mode_is_charge())
            sysoff_shutdown();

out:
    return ret;
}
AIC_COMP_L1_INIT_EXPORT(bcd_init);
#endif
