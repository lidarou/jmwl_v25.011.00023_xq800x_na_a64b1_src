/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __DMA_BOARD_CFG_H__
#define __DMA_BOARD_CFG_H__

#include <reg_base.h>
#include <irq.h>
#include <top_glb.h>
#include <aon_glb.h>
#include <mcu_glb.h>

struct aic_dma_board_cfg {
    u32 index;
    u32 base;
    u32 glb_base;
    u32 glb_bit;
    u32 irq_no;
    u32 chn;
};

static struct aic_dma_board_cfg dma_bd_cfg[1] = {
    {
        .index = 0,
        .base = REG_BASE_AP_DMAC,
        .glb_base = REG_MCU_GLB_DMA_EB_NOSEC,
        .glb_bit = BIT_MCU_GLB_DMA_EB_NOSEC,
        .irq_no = IRQ_DMA,
        .chn = DMA_CHN_MAX,
    },
};

#endif /* __DMA_BOARD_CFG_H__ */

