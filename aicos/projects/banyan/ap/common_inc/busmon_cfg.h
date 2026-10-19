/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __BUSMON_BOARD_CFG_H__
#define __BUSMON_BOARD_CFG_H__

#include <aic_auto_reg.h>
#include <aic_reg_base.h>
#include <bus_cfg_mcu_main.h>
#include <irq_chip.h>
#include <busmon_inc.h>

static struct aic_busmon_board_cfg busmon_bd_cfg[] = {
    {
        .index = BM_MCU,
        .base = REG_BASE_BLK_MCU_MCU_BUS_MONITOR,
        .glb_base = REG_BUS_CFG_MCU_MAIN_EB_S3_2_3_11_MCU_BUS_MONITOR,
        .glb_bit = BIT_BUS_CFG_MCU_MAIN_EB_S3_2_3_11_MCU_BUS_MONITOR,
        .irq_no = IRQ_BM,
    },
};

struct gx_bm_def_mon {
    char *name;
    BUSMON_IP ip;
    BUSMON_SEL chn;
    u32 start;
    u32 end;
};

static struct gx_bm_def_mon gx_bm_mon[] = {
    {"Monitor CP ckm glb", BM_MCU, BM_MCU_S1, REG_BASE_CKM_EN_GLB_CP, REG_BASE_CKM_EN_GLB_CP + 0xFFFF},
};

#endif /* __BUSMON_BOARD_CFG_H__ */

