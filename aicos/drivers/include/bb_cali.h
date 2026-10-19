/*****************************************************************************/
/*                                                                           */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines bb msrc cali interface.
 */

#ifndef __BB_CALI_H__
#define __BB_CALI_H__

#include <aic_type.h>

#ifdef CONFIG_AIC_BB_CALI
int msrc0_power_ctl(boolean on);
void aic_msrc0_cali_timer_enable(boolean enable);
#else
static inline int msrc0_power_ctl(boolean on)
{
    return AIC_SUCCESS;
}

static inline void aic_msrc0_cali_timer_enable(boolean enable)
{
    return;
}
#endif

#endif

