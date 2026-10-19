/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  chgr dummy driver file.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0671a9
#define LOG_MODULE    AMOD_PMIC

#include <aic_errno.h>
#include <aic_io.h>
#include <aic_osa.h>
#include <aic_type.h>
#include <power_supply.h>

bool psy_is_charging_complete(void)
{
    return false;
}

int psy_clear_chg_complete_state(void)
{
    return 0;
}

int  psy_cc_current_set(u32 cc_current_mA)
{
    return 0;
}

int psy_cc_current_get(u32 *cc_current_mA)
{
    return 0;
}

bool psy_is_charging_enabled(void)
{
    return true;
}

int __psy_charging_enable(bool enabled)
{
    return 0;
}
