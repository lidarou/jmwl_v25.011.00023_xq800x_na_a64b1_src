/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines exception process.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1cb120
#define LOG_MODULE    AMOD_DRV

#include <aic_type.h>
#include <aic_bitmap.h>

boolean aic_undef_handler(u32 fault_addr, u32 spsr)
{
    return FALSE;
}

boolean aic_swi_handler(void)
{
    return FALSE;
}

boolean aic_pabt_handler(u32 fault_addr, u32 fault_status)
{
    return FALSE;
}

boolean aic_dabt_handler(u32 fault_addr, u32 fault_status)
{
    return FALSE;
}
