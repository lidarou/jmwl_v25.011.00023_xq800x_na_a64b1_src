/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the lpr phy driver.
*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x106e4c
#define LOG_MODULE    AMOD_PMIC

#include <aic_bitmap.h>
#include <aic_cmdline.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_errno.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <aic_osa.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include "../lpr_phy.h"

#define LONG_PRESS_MAX_TIMING  14

/*key gnd keep connent status config*/
#define PWR_KEY_TIMER       "PWR_KEY_TIMER"
#define PWR_KEY_EXPIR_TIME  2000
#define PWR_KEY_GND_TASK_PRIO        3
#define LPR_MAX_CNT         0x7

struct lpr_phy_ops *cur_lpr_phy_ops = NULL;
static int lpr_max_timing = LONG_PRESS_MAX_TIMING;

int gp201_lpr_phy_config(BOOL enabled, u32 effect_time_s)
{
    u32 reg;
    u32 press_timing;

    reg = pmic_readl(REG_PMU_RTC_PMU_FSM_TRIGGER_REG1);

    if (!enabled) {
        reg &= ~BIT_PMU_RTC_EN_PWRKEY_SHUTDOWN_REG;
        goto out;
    }
    reg |= BIT_PMU_RTC_EN_PWRKEY_SHUTDOWN_REG;
    reg &= ~BIT_PMU_RTC_PWRKEY_SHUTDOWN_CNT_REG(0xf);

    /*PWRKEY longpress reset timing*/
    if (effect_time_s > LONG_PRESS_MAX_TIMING)
        press_timing = LONG_PRESS_MAX_TIMING;
    else
        press_timing = effect_time_s;

    reg |= BIT_PMU_RTC_PWRKEY_SHUTDOWN_CNT_REG(press_timing / 2);

out:
    pmic_writel(REG_PMU_RTC_PMU_FSM_TRIGGER_REG1, reg);

    return 0;
}

void gp201_disable_lpr(void)
{
    u32 reg;
    int pmic_id;
    
    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    if (BIT_GLB_PD_CHIP_ID_SUB_VERSION(pmic_id) > 0x4) {
        reg = pmic_readl(REG_PMU_RTC_PMU_FSM_TRIGGER_REG1);
        /* disable long press pwr key function */
        reg &= ~(BIT_PMU_RTC_EN_PWRKEY_SHUTDOWN_REG | BIT_PMU_RTC_PWRKEY_SHUTDOWN_MODE_REG);
        pmic_writel(0x238, 0x0);
        pmic_writel(REG_PMU_RTC_PMU_FSM_TRIGGER_REG1, reg);
    }
}

struct lpr_phy_ops gp201_lpr_phy_ops = {
    .p_lpr_phy_config = gp201_lpr_phy_config,
    .p_disable_lpr = gp201_disable_lpr,
};

#define PWR_KEY_RST_BB_EN                0x434
#define GP202_LONG_PRESS_MAX_TIMING      30
#define GP202_PWR_KEY_SHUTDOWN_CNT_REG   0x424

int gp202_lpr_phy_config(BOOL enabled, u32 effect_time_s)
{
    u32 reg;
    u32 press_timing;

    if (!enabled) {
        pmic_writel(PWR_KEY_RST_BB_EN, 0x0);
        goto out;
    }

    /*PWRKEY longpress reset timing*/
    if (effect_time_s > lpr_max_timing)
        press_timing = lpr_max_timing;
    else
        press_timing = effect_time_s;

    reg = (press_timing / 2) & 0xF;
    pmic_writel(GP202_PWR_KEY_SHUTDOWN_CNT_REG, reg);
    pmic_writel(PWR_KEY_RST_BB_EN, 0x1);

out:
    return 0;
}

void gp202_disable_lpr(void)
{
    pmic_writel(PWR_KEY_RST_BB_EN, 0x0);
}

struct lpr_phy_ops gp202_lpr_phy_ops = {
    .p_lpr_phy_config = gp202_lpr_phy_config,
    .p_disable_lpr = gp202_disable_lpr,
};

int lpr_phy_config(BOOL enabled, u32 effect_time_s)
{
    if (cur_lpr_phy_ops)
        return cur_lpr_phy_ops->p_lpr_phy_config(enabled, effect_time_s);
    else
        return 0;
}

void disable_lpr(void)
{
    if (cur_lpr_phy_ops)
        return cur_lpr_phy_ops->p_disable_lpr();
    else
        return;
}

int lpr_phy_ops_init(void)
{
    int ret =0, pmic_id;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    if (((pmic_id >> 16) & 0xFFFF) == 0x202) {
        lpr_max_timing = GP202_LONG_PRESS_MAX_TIMING;
        cur_lpr_phy_ops = &gp202_lpr_phy_ops;
    } else {
        lpr_max_timing = LONG_PRESS_MAX_TIMING;
        cur_lpr_phy_ops = &gp201_lpr_phy_ops;
    }

    return ret;
}
AIC_DRV_P1_INIT_EXPORT(lpr_phy_ops_init);
