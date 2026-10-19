/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __PERF_BOARD_CFG_H__
#define __PERF_BOARD_CFG_H__

#include <aic_auto_reg.h>
#include <aic_reg_base.h>
#include <irq_chip.h>
#include <perf_inc.h>

#define GX_PERF_FREQ        198000000

struct gx_perf_board_cfg {
    PERF_IP index;
    u32 base;
    u32 glb_base;
    u32 glb_bit;
    u32 irq_no;
};

struct gx_perf_desc {
    char *name;
    PERF_IP ip;
    PERF_CHN chn;
    PERF_OP_TYPE op;
    u32 userid;
    PERF_ADDR_TYPE addr_type;
};

static struct gx_perf_board_cfg perf_bd_cfg[] = {
    {
        .index = PERF_MEM_M0,
        .base = REG_BASE_BUSMON_MEMCTRL_CP,
        .glb_base = REG_MEM_CTRL_MEM_CTRL_DBG_BUS_CFG0,
        .glb_bit = BIT_MEM_CTRL_AXI_BUSMON_EN(0x1),
        .irq_no = IRQ_BM_CP,
    },
    {
        .index = PERF_MEM_M1,
        .base = REG_BASE_BUSMON_MEMCTRL_MCU,
        .glb_base = REG_MEM_CTRL_MEM_CTRL_DBG_BUS_CFG0,
        .glb_bit = BIT_MEM_CTRL_AXI_BUSMON_EN(0x2),
        .irq_no = IRQ_BM_CPU,
    },
    {
        .index = PERF_MEM_M2,
        .base = REG_BASE_BUSMON_MEMCTRL_MODEM,
        .glb_base = REG_MEM_CTRL_MEM_CTRL_DBG_BUS_CFG0,
        .glb_bit = BIT_MEM_CTRL_AXI_BUSMON_EN(0x4),
        .irq_no = IRQ_BM_ACC,
    },
};

static struct gx_perf_desc gx_perf_info[] = {
    {"CP Read",         PERF_MEM_M0,    PERF_CHN0,  PERF_READ,      0x0,    PERF_PSRAM},
    {"CP Write",        PERF_MEM_M0,    PERF_CHN1,  PERF_WRITE,     0x0,    PERF_PSRAM},
    {"AP Read",         PERF_MEM_M1,    PERF_CHN0,  PERF_READ,      0x0,    PERF_PSRAM},
    {"AP Write",        PERF_MEM_M1,    PERF_CHN1,  PERF_WRITE,     0x0,    PERF_PSRAM},
    {"Modem Read",      PERF_MEM_M2,    PERF_CHN0,  PERF_READ,      0x0,    PERF_PSRAM},
    {"Modem Write",     PERF_MEM_M2,    PERF_CHN1,  PERF_WRITE,     0x0,    PERF_PSRAM},
};

#endif /* __PERF_BOARD_CFG_H__ */

