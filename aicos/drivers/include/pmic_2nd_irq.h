/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software of pmic 2nd interrupt module.
 *   It includes all the pmic 2nd int hal common function api.
*/
#ifndef __PMIC_INT_2ND_H__
#define __PMIC_INT_2ND_H__
#include <pmic_interrupt.h>

#define PMIC_PMU_IRQ_MSK (BIT(PMIC_IRQ_PMU_RTC_INT) | BIT(PMIC_IRQ_PMU_UPS_INT) | BIT(PMIC_IRQ_PMU_PD_INT))

int pmic_irq_msk_append(int irq_msk);
int pmic_irq_prior_num_get(int irq_num_2nd);
void PMIC_2ND_IRQEnable(u32 irq_num);
void PMIC_2ND_IRQDisable(u32 irq_num);
void PMIC_2ND_IRQDisAll(void);
void PMIC_2ND_IRQClrAll(void);
int PMIC_2ND_GET_IRQ_ENSTATUS(int irq);

#endif /* __PMIC_INT_2ND_H__ */