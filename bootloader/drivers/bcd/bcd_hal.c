/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  battery charging device node operate handle file.
 *
 */

#include <aic_errno.h>
#include <aic_size.h>
#include <aic_type.h>
#include <bcd.h>
#include "bcd_core.h"
#include <initcall.h>
#include "console/console.h"

int bcd_detect(void)
{
    int ret = AIC_ENODEV;

#ifdef CONFIG_BCD_DUMMY
    if (aic_chg_int_rise())
        ret = AIC_BC_TYPE_SDP;
#else
    if (aic_chg_int_rise())
        ret = aic_bcd_result();
#endif

    return ret;
}
