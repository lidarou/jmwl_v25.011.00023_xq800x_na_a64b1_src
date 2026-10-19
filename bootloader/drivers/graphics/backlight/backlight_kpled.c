/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the kpled operate api.
*/
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_type.h>
#include <console/console.h>
#include <initcall.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>

int aic_bl_brightness_set(u32 brt_perc)
{
    u32 reg;

    reg = pmic_readl(REG_PMU_PD_LED_REG0);

    reg |= BIT_PMU_PD_REG1_LED1_18(0xF);

    if (brt_perc)
        reg &= ~BIT_PMU_PD_LED1_PD_18_REG;
    else
        reg |= BIT_PMU_PD_LED1_PD_18_REG;

    return pmic_writel(REG_PMU_PD_LED_REG0, reg);
}