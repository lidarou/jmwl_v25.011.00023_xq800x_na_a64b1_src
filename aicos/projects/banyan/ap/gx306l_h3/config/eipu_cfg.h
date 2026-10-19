/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __EIPU_BOARD_CFG_H__
#define __EIPU_BOARD_CFG_H__

#include <reg_base.h>
#include <irq.h>
#include <top_glb.h>
#include <aon_glb.h>
#include <mcu_glb.h>
#include <ckm_en_glb_aon.h>

struct aic_eipu_board_cfg {
    EIPU_MODE_TYPE_E type;
    u32 base;
    u32 aon_eb;
    u32 rst;
    u32 irq_no;
    u32 chn;
};

static struct aic_eipu_board_cfg eipu_bd_cfg[2] = {
    {
        .type = ND_MODE,
        .base = REG_BASE_TOP_ND_EIPU,
        .aon_eb = (REG_BASE_TOP_ND_CRU_TOP + 0x30),
        .rst = (REG_BASE_TOP_ND_CRU_TOP + 0x4c),
        .irq_no = IRQ_ND_EIPU,
        .chn = 3,
    },

    {
        .type = NOSEC_MODE,
        .base = REG_BASE_TOP_AON_EIPU,
        .aon_eb = REG_CKM_EN_GLB_AON_AON_EIPU_MCU_EB,
        .rst = REG_CKM_EN_GLB_AON_AON_EIPU_MCU_SW_RST_GLB_CFG,
        .irq_no = IRQ_AON_EIPU,
        .chn = 6,
    },
};

#endif /* __EIPU_BOARD_CFG_H__ */

