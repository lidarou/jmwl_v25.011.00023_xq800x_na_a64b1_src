/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines configuration items of pmic interrupt controller.
*/

#ifndef __PMIC_IRQ_H__
#define __PMIC_IRQ_H__

#include <pmic_interrupt.h>

typedef void (*PMIC_ISR_FUNC)(void *param);

typedef struct pmic_int_config {
    PMIC_ISR_FUNC pmic_isr_f;
    void *uparam;
} PMIC_INT_CONFIG_T;

/**
 * register_pmic_irq() - Register a pmic interrupt.
 *
 * @int_num: Pmic Interrupt number to be registered
 * @isr_f: PMIC ISR function
 * @uparam: Parameters for upper PMIC ISR
 *
 * Register a pmic interrupt. If new int registered which already exist,
 * The old one will be replaced.
 *
 * Return: AIC_EINVAL if int number or handler not valid.
 *
 */
int register_pmic_irq(PMIC_IRQn_Type int_num,
                 PMIC_ISR_FUNC isr_f,
                 void *uparam);

/**
 * register_pmic_irq() - Unregister an pmic interrupts
 *
 * Just clear pmic_isr_f and uparam.
 *
 *
 */
void unregister_pmic_irq(PMIC_IRQn_Type int_num);

/**
 * unregister_pmic_irq_all() - Unregister all pmic interrupts
 *
 * Just clear pmic_isr_f and uparam.
 *
 *
 */
void unregister_pmic_irq_all(void);

#endif /* __PMIC_IRQ_H__ */
