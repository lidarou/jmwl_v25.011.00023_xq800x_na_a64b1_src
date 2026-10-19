/*****************************************************************************/
/*                                                                           */
/*   Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is chip_id api file.
 *
*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x08581d
#define LOG_MODULE    AMOD_DRV

#include <aic_osa.h>
#include <aic_auto_reg.h>
#include <aic_reg_base.h>
#include <aic_io.h>
#include <aic_type.h>
#include <aic_errno.h>
#include <aic_init.h>

u32 get_chip_id(void)
{
    return AIC_READ32(REG_CHIP_ID);
}

u32 get_version_id(void)
{
    return AIC_READ32(REG_VERSION_ID);
}