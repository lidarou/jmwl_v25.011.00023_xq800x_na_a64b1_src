/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * pmic irq processing file.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x107234
#define LOG_MODULE    AMOD_PMIC

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aic_type.h>
#include <aic_osa.h>
#include <cache.h>
#include <pmic_irq.h>
#include <stdlib.h>
#include <irq_chip.h>
#include "pmic_ipu.h"
#include "pmic_2nd_irq.h"
#include <core_star.h>
#include <core_op.h>
#include <slp_inc.h>

PMIC_INT_CONFIG_T g_pmic_irq_config[PMIC_INT_MAXIUM] = {0};
/**
 * register_pmic_irq() - Register a pmic interrupt.
 *
 * @int_num: pmic Interrupt number to be registered
 * @isr_f: ISR function
 * @uparam: Parameters for upper ISR
 *
 * Register a interrupt. If new int registered which already exist,
 * The old one will be replaced.
 *
 * Return: AIC_EINVAL if int number or handler not valid.
 *
 */
int register_pmic_irq(PMIC_IRQn_Type int_num,
                 PMIC_ISR_FUNC isr_f,
                 void *uparam)
{
    int pmic_irq_num_1st;

    if (int_num >= PMIC_INT_MAXIUM || int_num < 0 || !isr_f) {
        AOS_IDXPR(0x17e904, "", "pmic int num error or isr is null!\n");
        return AIC_EINVAL;
    }

    g_pmic_irq_config[int_num].pmic_isr_f= isr_f;
    g_pmic_irq_config[int_num].uparam = uparam;

    if (int_num < PMIC_2ND_INT_START) {
        PMIC_IPU_IRQEnable(int_num);
    } else {
        PMIC_2ND_IRQEnable(int_num);
        pmic_irq_num_1st = pmic_irq_prior_num_get(int_num);
        if (pmic_irq_num_1st < 0)
            return AIC_ESRCH;
        PMIC_IPU_IRQEnable(pmic_irq_num_1st);
    }

    return AIC_SUCCESS;
}

/**
 * pmic_irq_handler() - pmic_irq handler
 *
 * Execute ISR, and send msg to ISR task if necessary.
 *
 */
void pmic_irq_handler_bisr(void *val)
{
    PMIC_IRQn_Type int_num;
    PMIC_ISR_FUNC isr_f;
    void *param;
    int value;

    aic_get_wakelock();

    /* Get exception number */
    value = PMIC_IPU_GetMaskStatus();

    if(value & PMIC_PMU_IRQ_MSK)
        value = pmic_irq_msk_append(value);

    while (value) {
        int_num = aic_find_first_bit(value);
        value &= ~BIT(int_num);

        if (int_num >= PMIC_INT_MAXIUM)
            continue;

        isr_f = g_pmic_irq_config[int_num].pmic_isr_f;
        param = g_pmic_irq_config[int_num].uparam;
        AIC_ASSERT(isr_f != NULL);

        if (param)
            isr_f(param);
        else
            isr_f((void *)int_num);
        NVIC_EnableIRQ(IRQ_PMIC);
    }
    NVIC_EnableIRQ(IRQ_PMIC);

    aic_put_wakelock();
}

/**
 * unregister_pmic_irq() - Unregister an interrupts
 *
 * Just clear isr_f and uparam.
 *
 *
 */
void unregister_pmic_irq(PMIC_IRQn_Type int_num)
{
    int pmic_irq_num_1st;

    if (int_num < PMIC_INT_MAXIUM) {
        if (int_num < PMIC_2ND_INT_START) {
            PMIC_IPU_IRQDisable(int_num);
        } else {
            PMIC_2ND_IRQDisable(int_num);
            if(!PMIC_2ND_GET_IRQ_ENSTATUS(int_num)) {
                pmic_irq_num_1st = pmic_irq_prior_num_get(int_num);
                if (pmic_irq_num_1st < 0)
                    goto out;

                PMIC_IPU_IRQDisable(pmic_irq_num_1st);
            }
        }
out:
        g_pmic_irq_config[int_num].pmic_isr_f = NULL;
        g_pmic_irq_config[int_num].uparam = NULL;
    }
}

/**
 * unregister_pmic_irq_all() - Unregister all interrupts
 *
 * Just clear isr_f and uparam.
 *
 *
 */
void unregister_pmic_irq_all(void)
{
    for (s32 i = 0; i < PMIC_INT_MAXIUM; i++) {
        if (i < PMIC_2ND_INT_START)
            PMIC_IPU_IRQDisable((PMIC_IRQn_Type)i);
        else
            PMIC_2ND_IRQDisable((PMIC_IRQn_Type)i);

        g_pmic_irq_config[i].pmic_isr_f = NULL;
        g_pmic_irq_config[i].uparam = NULL;
    }
}

int pmic_irq_handler_uisr(void *val)
{
    NVIC_DisableIRQ(IRQ_PMIC);
    NVIC_ClearPendingIRQ(IRQ_PMIC);

    return CALL_HWORKER;
}

/**
 * pmic_irq_init() - Initialize interrupt
 *
 * Disable All interrupt and clear pending signals.
 *
 */
int pmic_irq_init(void)
{
    /*disable all int then register pmic int to nvic*/
    PMIC_IPU_IRQDisAll();
    PMIC_2ND_IRQDisAll();
    PMIC_2ND_IRQClrAll();

    aos_request_irq_with_half_bottom(IRQ_PMIC, pmic_irq_handler_uisr,
                                    pmic_irq_handler_bisr, (void *)NULL);

    return AIC_SUCCESS;
}
AIC_COMP_P1_INIT_EXPORT(pmic_irq_init);
