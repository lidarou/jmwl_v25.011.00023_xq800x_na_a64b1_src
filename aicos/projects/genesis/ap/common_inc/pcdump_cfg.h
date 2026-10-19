/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __PCDUMP_BOARD_CFG_H__
#define __PCDUMP_BOARD_CFG_H__

#include <aic_auto_reg.h>
#include <aic_reg_base.h>

struct aic_pcdump_board_cfg {
    u32 base;
    u32 glb_base;
    u32 glb_bit;
    u32 trg_base;
    u32 trg_bit;
    u32 base_cp_sts;
    u32 cp_sts_flag;
};

static struct aic_pcdump_board_cfg pcdump_bd_cfg = {
    .base        = REG_BASE_AP_CP_PCDUMP,
    .glb_base    = REG_CPU_GLB_CR5_PCDUMP_EB,
    .glb_bit     = BIT_CPU_GLB_PCDUMP_EB,
    .trg_base    = REG_CPU_GLB_CR5_PCDUMP,
    .trg_bit     = BIT_CPU_GLB_PCDUMP_TRIGGER,
    .base_cp_sts = REG_PMU_CTRL_PD_CP_STATE,
    .cp_sts_flag = BIT_PMU_CTRL_PD_CP_STATE(0x1F),
};

#endif /* __PCDUMP_BOARD_CFG_H__ */

