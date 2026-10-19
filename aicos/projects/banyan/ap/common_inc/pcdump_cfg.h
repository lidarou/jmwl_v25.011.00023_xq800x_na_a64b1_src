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
    .base        = REG_BASE_BLK_MCU_PCDUMP,
    .glb_base    = REG_CKM_EN_GLB_MCU_CR5_PCDUMP_EB,
    .glb_bit     = BIT_CKM_EN_GLB_MCU_CR5_PCDUMP_EB,
    .trg_base    = REG_CKM_EN_GLB_MCU_CR5_PCDUMP_STATE,
    .trg_bit     = BIT_CKM_EN_GLB_MCU_PCDUMP_TRIGGER,
    .base_cp_sts = REG_AON_PMU_PWR_STATUS_CP,
    .cp_sts_flag = BIT_AON_PMU_PD_STATE_CP(0x1F),
};

#endif /* __PCDUMP_BOARD_CFG_H__ */

