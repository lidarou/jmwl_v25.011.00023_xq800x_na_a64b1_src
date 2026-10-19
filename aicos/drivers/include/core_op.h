/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines cpu core interface.
*/

#ifndef __CORE_OP_H__
#define __CORE_OP_H__

#if defined(CONFIG_CPU_CORTEX_R5)
#include <arch/arm/cortex-r5/core_r5.h>
#define AIC_GET_INT_STAUTS              interrupt_save = get_cpsr();
#endif

#if defined(CONFIG_CPU_CORTEX_MSTAR) || defined(CONFIG_CPU_CORTEX_M7)
#include <cmsis_compiler.h>
#define AIC_GET_INT_STAUTS              interrupt_save = __get_PRIMASK();
#endif

#define AIC_INT_ENABLE                  enable_interrupts();

#ifdef CONFIG_AOS_INT_DEBUG
#include <aic_osa.h>

#define AIC_INT_SAVE_AREA               AOS_INTERRUPT_DECLARE
#define AIC_INT_DISABLE                 AOS_INTERRUPT_DISABLE
#define AIC_INT_RESTORE                 AOS_INTERRUPT_RESTORE
#else
#define AIC_INT_SAVE_AREA               unsigned int interrupt_save;
#define AIC_INT_DISABLE                 interrupt_save = disable_interrupts();
#define AIC_INT_RESTORE                 restore_interrupts(interrupt_save);
#endif

#endif /* __CORE_OP_H__ */
