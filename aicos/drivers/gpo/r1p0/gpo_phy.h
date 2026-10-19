/*****************************************************************************/
/*                                                                           */
/*   Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of gpo module, it is gpo
 *    phy driver header file.
 */

void aic_gpo_phy_output(u32 gpo_id, GPO_MODE mode);
GPO_MODE aic_gpo_phy_get_level(u32 gpo_id);
void aic_gpo_phy_bare_cfg(u32 pin_base, GPO_MODE mode);
void aic_gpo_phy_init(void);

