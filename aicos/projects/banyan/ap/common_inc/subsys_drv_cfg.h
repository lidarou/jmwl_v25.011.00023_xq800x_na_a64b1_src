/*****************************************************************************/
/*                                                                           */
/*         Copyright 2023 by AIC Micro Corporation. All rights reserved.     */
/*                                                                           */
/*****************************************************************************/

#ifdef __SUBSYS_DRV_CFG_H__
#error redefine __SUBSYS_DRV_CFG_H__
#else
#define __SUBSYS_DRV_CFG_H__

#include <aon_pmu.h>

#define POWER_ON_STATE          0x00
#define POWER_OFF_STATE         0x1f

static void subsys_cp_hold_sys(void)
{
    AIC_REG_SETBIT(REG_AON_PMU_CP_SOFT_RESET_REQ, BIT_AON_PMU_SOFT_RESET_REQ_CP);
}

static void subsys_cp_release_sys(void)
{
    AIC_REG_CLRBIT(REG_AON_PMU_CP_SOFT_RESET_REQ, BIT_AON_PMU_SOFT_RESET_REQ_CP);
}

static void subsys_cp_pwron(void)
{
    /* clear BIT_AON_PMU_DSLP_AUTOMODE_SEL_CP to sel force deep. */
    AIC_WRITE32(REG_AON_PMU_CP_PWR_CTRL_CLR, BIT_AON_PMU_DSLP_AUTOMODE_SEL_CP);

    /* clear cp force deep sleep to poweron cp. */
    AIC_WRITE32(REG_AON_PMU_CP_PWR_CTRL_CLR, BIT_AON_PMU_DSLP_FRC_CP);
                ;

    /* wait power on state */
    while (POWER_ON_STATE != BIT_AON_PMU_PD_STATE_CP(AIC_READ32(REG_AON_PMU_PWR_STATUS_CP)));
}

static void subsys_cp_pwroff(void)
{
    subsys_cp_hold_sys();

    /* clear BIT_AON_PMU_DSLP_AUTOMODE_SEL_CP to sel force deep. */
    AIC_WRITE32(REG_AON_PMU_CP_PWR_CTRL_CLR, BIT_AON_PMU_DSLP_AUTOMODE_SEL_CP);

    /* set cp force deep sleep to poweron cp. */
    AIC_WRITE32(REG_AON_PMU_CP_PWR_CTRL_SET, BIT_AON_PMU_DSLP_FRC_CP);
}

static bool subsys_cp_is_power_off(void)
{
    return (POWER_OFF_STATE == BIT_AON_PMU_PD_STATE_CP(AIC_READ32(REG_AON_PMU_PWR_STATUS_CP)));
}

#endif /* __SUBSYS_DRV_CFG_H__ */

