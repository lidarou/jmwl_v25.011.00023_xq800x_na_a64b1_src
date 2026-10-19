/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of dma module, it is dma
 *    phy driver header file.
*/

#ifndef _DMA_INTERFACE_H_
#define _DMA_INTERFACE_H_

#include <aic_reg_base.h>
#include <aic_type.h>
#include <aic_dma_chn.h>
#include <dma.h>

void aic_dma_phy_chn_enable(u32 chn);
void aic_dma_phy_chn_disable(u32 chn);
void aic_dma_phy_chn_int_cfg(u32 chn, DMA_INT_TYPE type);
void aic_dma_phy_chn_ll_enable(u32 chn);
void aic_dma_phy_chn_set_hwid(u32 chn, DMA_HWID id);
void aic_dma_phy_chn_unset_hwid(u32 chn, DMA_HWID id);
void aic_dma_phy_resume(void);
int aic_dma_phy_suspend(void);
void aic_dma_phy_chn_resume(u32 chn);
int aic_dma_phy_chn_suspend(u32 chn);
void aic_dma_phy_chn_polling_done(u32 chn);
u32 aic_dma_phy_chn_get_src(u32 chn);
u32 aic_dma_phy_chn_get_dst(u32 chn);
void aic_dma_phy_chn_start(u32 chn);
int aic_dma_phy_chn_cfg(u32 chn, struct dma_chn_cfg *cfg);
int aic_dma_phy_ll_cfg(u32 chn, u32 llp, struct dma_chn_cfg *cfg, u32 node_cnt);
void aic_dma_phy_common_init(void);
u32 aic_dma_phy_get_int(void);
void aic_dma_phy_clr_int(u32 chn);
u32 aic_dma_phy_chn_int_status(u32 chn);
void aic_dma_phy_module_request(u32 index);
void aic_dma_phy_bare_cpy(u32 dst, u32 src, u32 size);

#endif /* _DMA_INTERFACE_H_ */

