/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 * pwr key gnd init file.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x10a436
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

#define PWR_KEY_TIMER       "PWR_KEY_TIMER"
#define PWR_KEY_EXPIR_TIME  2000

#define PWR_KEY_GND_TASK_PRIO        3
static aos_thread_id pwr_gnd_task_id = AOS_INVALID_THREAD_ID;

void pwr_key_gnd_init(void *param)
{
    if (aos_is_thread_created(pwr_gnd_task_id)) {
        aos_terminate_thread(pwr_gnd_task_id);
        aos_delete_thread(pwr_gnd_task_id);
    }
}

static void pwr_key_gnd_task_entry(void *data)
{
    u32 reg;

    /*make sure pmic pbint press cnt more than 2000ms*/
    aos_sleep(PWR_KEY_EXPIR_TIME);

    reg = pmic_readl(REG_PMU_RTC_PMU_FSM_TRIGGER_REG1);
    reg &= ~BIT_PMU_RTC_PWRKEY_SHUTDOWN_CNT_REG(0xf);
    /*PWRKEY longpress 2s reset*/
    reg |= BIT_PMU_RTC_PWRKEY_SHUTDOWN_CNT_REG(0x0);
    /* disable long press pwr key function */
    reg &= ~(BIT_PMU_RTC_EN_PWRKEY_SHUTDOWN_REG | BIT_PMU_RTC_PWRKEY_SHUTDOWN_MODE_REG);

    pmic_writel(REG_PMU_RTC_PMU_FSM_TRIGGER_REG1, reg);

}

int pwr_key_init(void)
{
    aos_timer_ptr pwr_key_gnd_timer;

    pwr_gnd_task_id = aos_create_thread_simply("pwr_key_gnd_task",
                                                 pwr_key_gnd_task_entry,
                                                 NULL, SIZE_1K, PWR_KEY_GND_TASK_PRIO);
    AIC_ASSERT(pwr_gnd_task_id != AOS_INVALID_THREAD_ID);

    pwr_key_gnd_timer = aos_create_timer(PWR_KEY_TIMER,
                                         pwr_key_gnd_init, NULL,
                                         PWR_KEY_EXPIR_TIME * 2,
                                         AOS_AUTO_ACTIVATE, AOS_NO_PERIOD);
    AIC_ASSERT(pwr_key_gnd_timer);

    return AIC_SUCCESS;
}
AIC_DRV_P1_INIT_EXPORT(pwr_key_init);
