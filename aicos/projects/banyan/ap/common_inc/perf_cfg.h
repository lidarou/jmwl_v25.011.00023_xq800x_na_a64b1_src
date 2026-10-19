/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __PERF_BOARD_CFG_H__
#define __PERF_BOARD_CFG_H__

#include <aic_auto_reg.h>
#include <aic_reg_base.h>
#include <irq_chip.h>
#include <perf_inc.h>

#define GX_PERF_FREQ        192000000
#define GX_PERF_CHN_MAX     2

struct gx_perf_board_cfg {
    PERF_IP index;
    u32 base;
    u32 glb_base[8];
    u32 glb_bit;
    u32 irq_no;
};

struct gx_perf_desc {
    char *name;
    PERF_CHN chn;
    PERF_OP_TYPE op;
    u32 userid;
    PERF_ADDR_TYPE addr_type;
};

static struct gx_perf_board_cfg perf_bd_cfg[] = {
    {
        .index = 0,
        .base = REG_BASE_BLK_MEM_CTRL_PM,
        .glb_base[0] = REG_CKM_EN_GLB_TOP_AXI_PERF_M0_EB,
        .glb_base[1] = REG_CKM_EN_GLB_TOP_AXI_PERF_M1_EB,
        .glb_base[2] = REG_CKM_EN_GLB_TOP_AXI_PERF_M2_EB,
        .glb_bit = 0x1,
        .irq_no = IRQ_MEM_AXI_PERF,
    },
};

static struct gx_perf_desc gx_perf_info[] = {
    {"CP Read",         PERF_CHN0,  PERF_READ,      0x0,    PERF_PSRAM},
    {"CP Write",        PERF_CHN0,  PERF_WRITE,     0x0,    PERF_PSRAM},
    //{"AP Read",         PERF_CHN1,  PERF_READ,      0x0,    PERF_PSRAM},
    //{"AP Write",        PERF_CHN1,  PERF_WRITE,     0x0,    PERF_PSRAM},
    //{"Modem Read",      PERF_CHN2,  PERF_READ,      0x0,    PERF_PSRAM},
    //{"Modem Write",     PERF_CHN2,  PERF_WRITE,     0x0,    PERF_PSRAM},
};

#endif /* __PERF_BOARD_CFG_H__ */

