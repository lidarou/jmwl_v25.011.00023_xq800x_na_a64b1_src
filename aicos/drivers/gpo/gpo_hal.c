/*****************************************************************************/
/*                                                                           */
/*   Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of gpo module, it is gpo
 *    common hal driver.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x13d50e
#define LOG_MODULE    AMOD_DRV

#include <aic_init.h>
#include <aic_type.h>

#include <gpo_inc.h>
#include <gpo_phy.h>

/*
 * aic_gpo_output
 * Config pin as gpo mode, and output some level.
 * @pin: the pin which needs to be config as gpo mode, pin should be id number
 * @mode: the pin output mode
 */
void aic_gpo_output(u32 gpo_id, GPO_MODE mode)
{
    aic_gpo_phy_output(gpo_id, mode);
}

/*
 * aic_gpo_get_level
 * Use to get pin gpo mode
 * @pin: the pin which needs to be read, pin should be id number
 * return val: gpo level mode
 */
GPO_MODE aic_gpo_get_level(u32 gpo_id)
{

    return aic_gpo_phy_get_level(gpo_id);
}

/*
 * aic_gpo_pin_bare_cfg
 * It is special api, maybe the pin has not gpo id, only base addr.
 * @pin_base: the pin which needs to be config as gpo mode, pin_base should be
 * pin base address.
 * @mode: the pin output mode
 */
void aic_gpo_pin_bare_cfg(u32 pin_base, GPO_MODE mode)
{
    aic_gpo_phy_bare_cfg(pin_base, mode);
}

int aic_gpo_init(void)
{
    aic_gpo_phy_init();

    return 0;
}

AIC_DRV_P1_INIT_EXPORT(aic_gpo_init);

