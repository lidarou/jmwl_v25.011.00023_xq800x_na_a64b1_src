/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of lpr module.
 *   It includes all the lpr phy function api.
*/
#ifndef __LPR_PHY_H__
#define __LPR_PHY_H__
#include <aic_bitmap.h>

struct lpr_phy_ops {
    int (*p_lpr_phy_config)(BOOL enabled, u32 effect_time_s);
    void (*p_disable_lpr)(void);
};

int lpr_phy_config(BOOL enabled, u32 effect_time_s);
void disable_lpr(void);

#endif /* __LPR_PHY_H__ */
