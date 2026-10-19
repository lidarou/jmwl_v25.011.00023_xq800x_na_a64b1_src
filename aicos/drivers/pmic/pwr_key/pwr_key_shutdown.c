/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * pmic pwr key file.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1bfde0
#define LOG_MODULE    AMOD_PMIC

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aic_type.h>
#include <aic_osa.h>
#include <pmic_auto_reg.h>
#include <pmic_irq.h>
#include <pmic_io.h>
#include <timer_inc.h>
#include <sysoff.h>

void pmic_pwrkey_press(void *param)
{
    aos_sleep(500);
    pmic_writel(REG_PMU_RTC_PMU_UPS_INT_CLR, BIT_PMU_RTC_PMU_UPS_INT_CLR(0x1));
    sysoff_shutdown();
    while(1)
        ;
}

/**
 * pmic_irq_init() - power key initialize.
 *
 */
int pmic_pwr_key_init(void)
{
    pmic_writel(REG_PMU_RTC_PMU_UPS_INT_CLR, BIT_PMU_RTC_PMU_UPS_INT_CLR(0x1));

    register_pmic_irq(PMIC_PMU_UPS_PWRKEY_PRESS, pmic_pwrkey_press, NULL);

    return AIC_SUCCESS;
}
AIC_APP_L2_INIT_EXPORT(pmic_pwr_key_init);
