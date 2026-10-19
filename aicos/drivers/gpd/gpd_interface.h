/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of gpd module, it is gpd
 *    phy driver header file.
*/

#ifndef _GPD_INTERFACE_H_
#define _GPD_INTERFACE_H_

#include <aic_reg_base.h>
#include <aic_type.h>
#include <gpd_inc.h>

#define GPD_CNT_CFG_MAX     50000

void aic_gpd_phy_enable(GPD_INDEX index);
void aic_gpd_phy_disable(GPD_INDEX index);
void aic_gpd_phy_update(GPD_INDEX index);
void aic_gpd_phy_cnt_clr(GPD_INDEX index);
void aic_gpd_phy_cycle_clr(GPD_INDEX index);
void aic_gpd_phy_common_init(void);
void aic_gpd_phy_edge_type_cfg(GPD_INDEX index, GPD_EDGE_TYPE type);
void aic_gpd_phy_glitch_set(GPD_INDEX index, u32 cnt);
void aic_gpd_phy_edge_cnt_set(GPD_INDEX index, u32 cnt);
u32 aic_gpd_phy_edge_cnt_get(GPD_INDEX index);
u32 aic_gpd_phy_cycle_cnt_get(GPD_INDEX index);
u32 aic_gpd_phy_get_int_state(void);
void aic_gpd_phy_int_clr(GPD_INDEX index);

#endif /* _GPD_INTERFACE_H_ */

