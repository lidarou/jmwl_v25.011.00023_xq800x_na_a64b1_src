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

#include "cmsis/cmsis_compiler.h"

#define AIC_INT_SAVE_AREA               unsigned int interrupt_save;
#define AIC_INT_DISABLE                 interrupt_save = disable_interrupts();
#define AIC_INT_RESTORE                 restore_interrupts(interrupt_save);

#endif /* __CORE_OP_H__ */
