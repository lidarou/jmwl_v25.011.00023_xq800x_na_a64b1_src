/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Physical interface for lzma HAL.
 *
 */

#ifndef __HLZMA_PHY_H__
#define __HLZMA_PHY_H__

#include <aic_type.h>

/* The struct used to pass setting items from hal to phy, it
 * may be extended for multi ip versions later.
 *
 */
typedef struct hlzma_settig_s {
    /* setting items of r1p0 */
    u32 in_addr;      /* compressed data start address */
    u32 out_addr;     /* decompressed data start address */
} hlzma_setting_t;

/* Enable or disable hardware lzma */
void hlzma_phy_en(BOOL en);

/* start lzma decompress processing */
s32 hlzma_phy_start(void);

/* reset lzma module */
s32 hlzma_phy_reset(void);

/* setting parameters */
s32 hlzma_phy_cfg(const hlzma_setting_t *cfg);

/* getting done */
s32 hlzma_phy_polling_done(void);

/* setting interrupt */
s32 hlzma_phy_int_cfg(void);

/* lzma interrupt handler */
void hlzma_int_handler(void *param);

/* Disable all interrupt after reset as all int are enabled after reset */
void hlzma_phy_int_disable(void);

#endif /* __HLZMA_PHY_H__ */
