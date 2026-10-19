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
#define AIC_THIS_FILE    0x061523
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
#include "pmic_2nd_phy.h"

struct pmic_2nd_irq_ops *pmic_2nd_ops;

int pmic_irq_msk_append(int irq_msk)
{
    if (pmic_2nd_ops->pmic_2nd_irq_msk_append)
        return pmic_2nd_ops->pmic_2nd_irq_msk_append(irq_msk);
    else
        return irq_msk;
}

int pmic_irq_prior_num_get(int irq_num_2nd)
{
    if (pmic_2nd_ops->pmic_1st_irq_num_get)
        return pmic_2nd_ops->pmic_1st_irq_num_get(irq_num_2nd);
    else
        return AIC_ENXIO;
}

void PMIC_2ND_IRQEnable(u32 irq_num)
{
    if (pmic_2nd_ops->pmic_2nd_irq_enable)
        pmic_2nd_ops->pmic_2nd_irq_enable(irq_num);
}

void PMIC_2ND_IRQDisable(u32 irq_num)
{
    if (pmic_2nd_ops->pmic_2nd_irq_disable)
        pmic_2nd_ops->pmic_2nd_irq_disable(irq_num);
}

void PMIC_2ND_IRQDisAll(void)
{
    if (pmic_2nd_ops->pmic_2nd_irq_disable_all)
        pmic_2nd_ops->pmic_2nd_irq_disable_all();
}

void PMIC_2ND_IRQClrAll(void)
{
    if (pmic_2nd_ops->pmic_2nd_irq_clear_all)
        pmic_2nd_ops->pmic_2nd_irq_clear_all();
}

int PMIC_2ND_GET_IRQ_ENSTATUS(int irq)
{
    if (pmic_2nd_ops->pmic_2nd_irq_en_sts_get)
        return (int)pmic_2nd_ops->pmic_2nd_irq_en_sts_get(irq);
    else
        return AIC_ENXIO;
}

void pmic_2nd_irq_ops_register(struct pmic_2nd_irq_ops *ops)
{
    pmic_2nd_ops = ops;
}
