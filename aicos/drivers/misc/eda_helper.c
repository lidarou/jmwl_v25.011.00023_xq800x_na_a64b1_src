/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Interface of eda helper.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0646e2
#define LOG_MODULE    AMOD_DRV

#include <aic_type.h>
#include <aic_errno.h>
#include <aic_io.h>

void eda_dbgreg_wr(u32 val)
{
#ifdef CONFIG_SUBSYS_CP
    AIC_WRITE32(0x41090084, val);
#endif
#ifdef CONFIG_SUBSYS_AP
    AIC_WRITE32(0x41090080, val);
#endif
}
