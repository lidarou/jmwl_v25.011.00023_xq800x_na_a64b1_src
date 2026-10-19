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
#define AIC_THIS_FILE    0x19bf98
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
#include <aic_factory.h>

#define CALI_STR    "cali"

aos_event_ptr pwr_press_event;
static aos_thread_id pwr_key_thread = AOS_INVALID_THREAD_ID;

#define PWR_KEY_TASK_PRIO   5
#define POWER_KEY_PRESS     0x1
static u32 pwrkey_sd_thold_cnt = (PWR_LONG_PRESS_SHUTDOWN_THRESHOLD * 1000) / 250;
#define LPR_MAX_CNT 0x3FFFC

#define USB_CHG_IN_BOOT             0x18
#define GP202_PWR_OFF_RST_FLAG_CLR  0x3C
#define GP202_PWR_KEY_PRESS_CNT     0x430
#define GP202_PWR_KEY_INT_CLR       0X43C

static u16 pmic_pwr_key_int_clr = REG_PMU_RTC_PMU_UPS_INT_CLR,
           pmic_pwr_key_press_cnt = REG_PMU_RTC_PMU_FSM_TRIGGER_REG2,
           pmic_pwr_off_rst_flag_clr = REG_GLB_UPS_PWR_OFF_RST_FLAG_CLR;

void pmic_pwrkey_handler(void *param)
{
    aos_set_event(pwr_press_event, POWER_KEY_PRESS, AOS_OR);
    /* clear int */
    pmic_writel(pmic_pwr_key_int_clr, BIT_PMU_RTC_PMU_UPS_INT_CLR(0x1));
}

static void pwr_key_thread_entry(void *data)
{
    u32 press_cnt_old = 0, press_cnt_new = 0;
    u32 pwr_press_flag;

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

    if (press_cnt_new > press_cnt_old) {
        aos_set_event(pwr_press_event, POWER_KEY_PRESS, AOS_OR);
    }
#endif

    while(1) {
            aos_get_event(pwr_press_event, POWER_KEY_PRESS, AOS_AND_CLEAR,
                    &pwr_press_flag, AOS_WAIT_FOREVER);
        while(1) {
            press_cnt_new = pmic_readl(pmic_pwr_key_press_cnt);
            if (press_cnt_new == press_cnt_old) {
#ifdef CONFIG_LPR
                /* either key is released or key pressed timeout (65.536s) */
                    lpr_config(true, PWR_LPR_TIMING_MAX);
                    lpr_state = LPR_MAX;
#endif
                break;
            } else {
#ifdef CONFIG_LPR
                /* escape LPR reset by setting compare value up and down */
                if ((press_cnt_new < (PWR_LPR_TIMING * 4000)) && (lpr_state != LPR_MAX)) {
                    lpr_config(true, PWR_LPR_TIMING_MAX);
                    lpr_state = LPR_MAX;
                } else if ((press_cnt_new >= (PWR_LPR_TIMING * 4000)) && (lpr_state != LPR_MIN)) {
                    lpr_config(true, PWR_LPR_TIMING_MIN);
                    lpr_state = LPR_MIN;
                }
#endif
                press_cnt_old = press_cnt_new;
            }
            aos_sleep(10);
        }
        press_cnt_old = 0;
    }
}

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
    int boot_src;
    u8 flag = 0;

    boot_src = aic_cmdline_get_value_int(cl_key_pwr_on);

    if ((boot_src & USB_CHG_IN_BOOT) == 0){
        lpr_config(true, 2);
        return 0;
    }

    if (!station_flag_get(CALI_STR, &flag))
        if (flag == 0x3) {
            lpr_config(true, PWR_LPR_TIMING_MIN);
            goto out;
        }

    pwrkey_sd_thold_cnt = LPR_MAX_CNT;

    pmic_writel(pmic_pwr_key_int_clr, BIT_PMU_RTC_PMU_UPS_INT_CLR(0x1));

    register_pmic_irq(PMIC_PMU_UPS_PWRKEY_PRESS, pmic_pwrkey_handler, NULL);

    pwr_press_event = aos_create_event("pwr_press_event");
    AIC_ASSERT(pwr_press_event);

    pwr_key_thread = aos_create_thread("pwr_key_thread", pwr_key_thread_entry, AOS_NULL, 1024, 8,
                      "pwr_key_thread", PWR_KEY_TASK_PRIO, AOS_AUTO_START);

    if (pwr_key_thread == AOS_INVALID_THREAD_ID) {
        aicx_error(0x146d13, "s", "%s create task fail!\n", __func__);
        return AIC_ENXIO;
    }

out:
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
