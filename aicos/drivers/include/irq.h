/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines configuration items of interrupt controller.
*/

#ifndef __IRQ_H__
#define __IRQ_H__

#include <aic_osa.h>
#include <irq_chip.h>

typedef void (*ISR_WRAPPER_FUNC)(void);

typedef ISR_RET_E (*UISR_FUNC)(void *param);

typedef void (*BISR_FUNC)(void *param);

typedef struct int_config {
    UISR_FUNC p_uisr;
    void * uparam;
    BISR_FUNC p_bisr;
    void * bparam;
    volatile u32 flag; /* 0,not enqueue;1,enqueued */
} INT_CONFIG_T;

/**
 * register_irq() - Register an interrupt.
 *
 * @no: Interrupt number to be registered
 * @uisr: Upper ISR
 * @uparam: Parameters for upper ISR
 * @bisr: Bottom ISR
 * @bparam: Parameters for bottom handler
 *
 * Register an interrupt and enable it. If new int registered which already exist,
 * The old one will be replaced.
 *
 * Return: AIC_EINVAL if int number or handler not valid.
 *
 */
s32 register_irq(IRQn_Type no,
                 UISR_FUNC uisr,
                 void *uparam,
                 BISR_FUNC bisr,
                 void *bparam);

/**
 * unregister_irq() - Unregister an interrupts
 *
 * Disable interrupt and clear its handler.
 *
 *
 */
void unregister_irq(IRQn_Type int_num);

/**
 * register_frq() - Register a fiq interrupt.
 *
 * @int_num: Interrupt number to be registered
 * @uisr: Upper ISR
 * @uparam: Parameters for upper ISR
 *
 * Register a fiq interrupt and enable it. If new int registered which already exist,
 * The old one will be replaced.
 *
 * Return: AIC_SUCCESS, if register success
 *         AIC_EINVAL, if upper handler is null or int number is too large
 *         AIC_EPERM, if get mutex failed
 *
 */
s32 register_frq(IRQn_Type int_num,
                 UISR_FUNC uisr,
                 void *uparam);

/**
 * irq_init() - Initialize interrupt
 *
 * Create 2 tasks to process bottom interrupt.
 * Disable All interrupt and clear pending signals.
 *
 */
s32 irq_init(void);

/**
 * irq_handler() - IRQ handler
 *
 * Execute ISR, and send msg to ISR task if necessary.
 *
 */
void irq_handler(void);

/**
 * irq_suspend() - Interrupt backup
 *
 * @addr: buffer address to save irq registers.
 *
 */
void irq_suspend(u32 *addr);

/**
 * irq_resume() - Interrupt restore
 *
 * @addr: buffer address to save irq registers.
 *
 */
void irq_resume(u32 *addr);

#endif /* __IRQ_H__ */
