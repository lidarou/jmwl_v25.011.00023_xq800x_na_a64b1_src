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

struct aic_eipu_board_cfg {
    EIPU_MODE_TYPE_E type;
    u32 base;
    u32 aon_eb;
    u32 rst;
    u32 irq_no;
    u32 chn;
};

static struct aic_eipu_board_cfg eipu_bd_cfg[1] = {
    {
        .type = NOSEC_MODE,
        .base = REG_BASE_AON_EIPU,
        .aon_eb = REG_AON_GLB_EIPU_EB,
        .rst = REG_TOP_GLB_SW_RST_AON_EIPU,
        .irq_no = IRQ_EIPU,
        .chn = EIPU_CHN_NUM,
    },
};

#endif /* __EIPU_BOARD_CFG_H__ */

