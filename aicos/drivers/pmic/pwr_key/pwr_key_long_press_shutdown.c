/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * pmic pwr key file.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0c54ca
#define LOG_MODULE    AMOD_PMIC

#include <aic_assert.h>
#include <aic_cmdline.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aic_type.h>
#include <aic_osa.h>
#include <pmic_auto_reg.h>
#include <pmic_irq.h>
#include <pmic_io.h>
#include <display.h>
#include <sysoff.h>
#include <pwr_key_cfg.h>
#include <aic_boot_mode.h>
#include <pwr_key.h>
#ifdef CONFIG_LPR
#include <lpr.h>
#endif
#include <bcd.h>

static LIST_HEAD(pwr_key_monitor_list);

struct pwr_key_monitor_dev {
    char *name;
    pwr_key_callback monitor_cb;
    struct list_head link_list;
};

aos_event_ptr pwr_press_event;
static aos_thread_id pwr_key_thread = AOS_INVALID_THREAD_ID;

#define PWR_KEY_TASK_PRIO   5
#define POWER_KEY_PRESS     0x1
static u32 pwrkey_sd_thold_cnt = (PWR_LONG_PRESS_SHUTDOWN_THRESHOLD * 1000) / 250;
#define LPR_MAX_CNT 0x3FFFC

#define GP202_PWR_OFF_RST_FLAG_CLR  0x3C
#define GP202_PWR_KEY_PRESS_CNT     0x430
#define GP202_PWR_KEY_INT_CLR       0X43C

#define GP201_REG_PMU_RTC_PMU_UPS_INT_CLR       0x9f4
#define GP201_REG_PMU_RTC_PMU_FSM_TRIGGER_REG2  0x984
#define GP201_REG_GLB_UPS_PWR_OFF_RST_FLAG_CLR  0x210
#define GP20X_BIT_PMU_RTC_PMU_UPS_INT_CLR(x)    (((x) & 0xf))

static u16 pmic_pwr_key_int_clr = GP201_REG_PMU_RTC_PMU_UPS_INT_CLR,
           pmic_pwr_key_press_cnt = GP201_REG_PMU_RTC_PMU_FSM_TRIGGER_REG2,
           pmic_pwr_off_rst_flag_clr = GP201_REG_GLB_UPS_PWR_OFF_RST_FLAG_CLR;

#ifdef CONFIG_FP
extern uint32_t lv_tick_elaps(uint32_t prev_tick);
extern uint32_t lv_tick_get(void);
#endif

void pmic_pwrkey_handler(void *param)
{
    aos_set_event(pwr_press_event, POWER_KEY_PRESS, AOS_OR);
    /* clear int */
    pmic_writel(pmic_pwr_key_int_clr, GP20X_BIT_PMU_RTC_PMU_UPS_INT_CLR(0x1));
}

void broadcast_pwr_key(AIC_PWR_KEY_STATUS pwr_key_status)
{
    struct pwr_key_monitor_dev *dev_ops;

    list_for_each_entry(dev_ops, &pwr_key_monitor_list,
                        link_list, struct pwr_key_monitor_dev) {
        dev_ops->monitor_cb((void *)&pwr_key_status, sizeof(AIC_PWR_KEY_STATUS));
    }
}

int register_pwr_key_monitor(char *monitor_name, pwr_key_callback callback)
{
    int ret = 0;
    struct pwr_key_monitor_dev *new_monitor_dev;

    new_monitor_dev = aos_zalloc(sizeof(struct pwr_key_monitor_dev));
    if (!new_monitor_dev) {
        ret = AIC_ENOSPC;
        aicx_warn(0x02ad96, "s", "%s not enough space!\n", __func__);
        goto out;
    }

    list_head_init(&new_monitor_dev->link_list);
    new_monitor_dev->monitor_cb = callback;
    new_monitor_dev->name = strdup(monitor_name);
    list_add(&new_monitor_dev->link_list, &pwr_key_monitor_list);

out:
    return ret;
}

int unregister_pwr_key_monitor(char *monitor_name)
{
    int ret = AIC_ENXIO;
    struct pwr_key_monitor_dev *dev_ops, *temp_ops;

    list_for_each_entry_safe(dev_ops, temp_ops, &pwr_key_monitor_list,
                            link_list, struct pwr_key_monitor_dev) {
        if (!strcmp(dev_ops->name, monitor_name)) {
            list_del(&dev_ops->link_list);
            ret = aos_free(dev_ops->name);
            ret |= aos_free(dev_ops);
            if (ret)
                aicx_warn(0x0be34b, "s", "%s free node fail!\n", __func__);
            break;
        }
    }

    return ret;
}

#ifdef CONFIG_FP
void pwr_key_power_off(void)
{
    pmic_writel(pmic_pwr_key_int_clr, GP20X_BIT_PMU_RTC_PMU_UPS_INT_CLR(0x1));
    pmic_writel(pmic_pwr_off_rst_flag_clr, 0xF);
    if(aic_boot_mode_is_charge())
        sysoff_emergency_reset();
    else
        sysoff_shutdown();
}
#endif

static void pwr_key_thread_entry(void *data)
{
    u32 press_cnt_old = 0, press_cnt_new = 0;
#ifdef CONFIG_FP
    u32 long_press_cnt = 0;
#endif
    u32 pwr_press_flag;
    bool key_poweron_press = false;
#ifdef CONFIG_LPR
    enum __lpr_state {
        LPR_MIN     = 0,
        LPR_DEFAULT = 1,
        LPR_MAX     = 2,
    };
    enum __lpr_state lpr_state = LPR_DEFAULT;

    press_cnt_old = pmic_readl(pmic_pwr_key_press_cnt);
    aos_sleep(2);
    press_cnt_new = pmic_readl(pmic_pwr_key_press_cnt);
    aicx_debug(0x07d76a, "dd", "PWRKEY: DETECT @ %d -> %d", press_cnt_old, press_cnt_new);
    if (press_cnt_new > press_cnt_old) {
        key_poweron_press = true;
    }
#endif

    while(1) {
        if (!key_poweron_press) {
            aos_get_event(pwr_press_event, POWER_KEY_PRESS, AOS_AND_CLEAR,
                    &pwr_press_flag, AOS_WAIT_FOREVER);
            broadcast_pwr_key(AIC_PWR_KEYDOWN);
#ifdef CONFIG_PWR_KEY_LINK_VBUS
            pwrkey_sd_thold_cnt = (PWR_LONG_PRESS_SHUTDOWN_THRESHOLD * 1000) / 250;
#endif
#ifdef CONFIG_FP
            long_press_cnt = lv_tick_get();
#endif
        }
        while(1) {
            press_cnt_new = pmic_readl(pmic_pwr_key_press_cnt);
            if (press_cnt_new == press_cnt_old) {
#ifdef CONFIG_LPR
                /* either key is released or key pressed timeout (65.536s) */
                if (!(PWR_LPR_ENABLED)) {
                    lpr_config(true, PWR_LPR_TIMING_MAX);
                    lpr_state = LPR_MAX;
                    aicx_debug(0x0ee978, "d", "PWRKEY: AIC_PWR_KEYUP @ %d", press_cnt_new);
                }
#endif
                if (key_poweron_press) {
                    key_poweron_press = false;
                } else {
                    broadcast_pwr_key(AIC_PWR_KEYUP);
                }
                break;
            } else {
#ifdef CONFIG_FP
            if(lv_tick_elaps(long_press_cnt) >= 2000)
            {
                if (!key_poweron_press)
                {
                    broadcast_pwr_key(AIC_PWR_KEYLONG);
                    long_press_cnt = lv_tick_get();
                }
                long_press_cnt = lv_tick_get();
            }
#endif
#ifdef CONFIG_LPR
        /* escape LPR reset by setting compare value up and down */
        if ((press_cnt_new < (PWR_LPR_TIMING * 4000)) && (lpr_state != LPR_MAX)) {
            lpr_config(true, PWR_LPR_TIMING_MAX);
            lpr_state = LPR_MAX;
            aicx_debug(0x1677d2, "d", "PWRKEY: PWR_LPR_TIMING_MAX @ %d", press_cnt_new);
        } else if ((press_cnt_new >= (PWR_LPR_TIMING * 4000)) && (lpr_state != LPR_MIN)) {
            lpr_config(true, PWR_LPR_TIMING_MIN);
            lpr_state = LPR_MIN;
            aicx_debug(0x18cc1b, "d", "PWRKEY: PWR_LPR_TIMING_MIN @ %d", press_cnt_new);
        }
#endif
            press_cnt_old = press_cnt_new;
#ifdef CONFIG_PWR_KEY_LINK_GND_QUICK_SLEEP
            if (press_cnt_new >= (PWR_LPR_TIMING * 2500)) {
                lpr_config(true, PWR_LPR_TIMING_MIN);
                key_poweron_press = false;
                break;
            }
#endif
        }

            if (!key_poweron_press
             && press_cnt_new > pwrkey_sd_thold_cnt
             && pwrkey_sd_thold_cnt != LPR_MAX_CNT) {
#ifdef CONFIG_PWR_KEY_LINK_VBUS
                if (aic_is_usb_plugin()) {
                    pwrkey_sd_thold_cnt = LPR_MAX_CNT;
                    aos_sleep(10);
                    continue;
                }
#endif
                pmic_writel(pmic_pwr_key_int_clr, GP20X_BIT_PMU_RTC_PMU_UPS_INT_CLR(0x1));
                pmic_writel(pmic_pwr_off_rst_flag_clr, 0xF);
                if(aic_boot_mode_is_charge())
                    sysoff_emergency_reset();
                else
                    sysoff_shutdown();
            }
            aos_sleep(10);
        }

        press_cnt_old = 0;
    }
}

#ifdef CONFIG_PWR_KEY_KICK_SWITCH_BACKLIGHT
int32_t pwr_key_bl_ctrl(void *para, uint32_t size)
{
    u32 bl = 0;
    int ret = 0;
    AIC_PWR_KEY_STATUS pwr_status = *(AIC_PWR_KEY_STATUS *)para;

    switch (pwr_status) {
    case AIC_PWR_KEYDOWN:
        aicx_warn(0x000e87, "s", "%s power keydown!\n", __func__);
        break;

    case AIC_PWR_KEYUP:
        aicx_warn(0x027e20, "s", "%s power keyup!\n", __func__);
        ret = aic_bl_brightness_get((u32 *)&bl);
        if (ret) {
            aicx_warn(0x082854, "s", "%s get backlight status fail!\n", __func__);
            break;
        }

        if (bl != 0) {
            ret = aic_bl_brightness_set(0);
            if (ret)
                break;
            display_suspend();
        } else {
            display_resume();
            ret = aic_bl_brightness_set(100);
        }
        break;

    default:
        aicx_warn(0x0db1b4, "sd", "%s unsupport pwr_key status:%d cb!\n", __func__, pwr_status);
        break;
    }

    return ret;
}
#endif

void pwrkey_sd_thold_cfg(u32 shutdown_thold_ms)
{
    pwrkey_sd_thold_cnt = (shutdown_thold_ms * 1000) / 250;
    if (pwrkey_sd_thold_cnt >= LPR_MAX_CNT)
        pwrkey_sd_thold_cnt = LPR_MAX_CNT;
}

/**
 * pmic_irq_init() - power key initialize.
 *
 */
int pmic_pwr_key_init(void)
{
    if (pwrkey_sd_thold_cnt >= LPR_MAX_CNT)
        pwrkey_sd_thold_cnt = LPR_MAX_CNT;

    pmic_writel(pmic_pwr_key_int_clr, GP20X_BIT_PMU_RTC_PMU_UPS_INT_CLR(0x1));

    register_pmic_irq(PMIC_PMU_UPS_PWRKEY_PRESS, pmic_pwrkey_handler, NULL);

    pwr_press_event = aos_create_event("pwrk");
    AIC_ASSERT(pwr_press_event);

    pwr_key_thread = aos_create_thread("pwrk", pwr_key_thread_entry, AOS_NULL, 1024, 0,
                      NULL, PWR_KEY_TASK_PRIO, AOS_AUTO_START);

    if (pwr_key_thread == AOS_INVALID_THREAD_ID) {
        aicx_error(0x146d13, "s", "%s create task fail!\n", __func__);
        return AIC_ENXIO;
    }

#ifdef CONFIG_PWR_KEY_KICK_SWITCH_BACKLIGHT
    register_pwr_key_monitor("pwr_key_backlight", pwr_key_bl_ctrl);
#endif

    return AIC_SUCCESS;
}
AIC_APP_L2_INIT_EXPORT(pmic_pwr_key_init);

int pwr_key_ops_reg_get(void)
{
    int pmic_id;

    pmic_id = (aic_cmdline_get_value_int(cl_key_pmic_version) >> 16) & 0xFFFF;
    if (pmic_id == 0x202) {
        pmic_pwr_key_int_clr = GP202_PWR_KEY_INT_CLR;
        pmic_pwr_key_press_cnt = GP202_PWR_KEY_PRESS_CNT;
        pmic_pwr_off_rst_flag_clr = GP202_PWR_OFF_RST_FLAG_CLR;
    }

    return 0;
}
AIC_DRV_P1_INIT_EXPORT(pwr_key_ops_reg_get);
