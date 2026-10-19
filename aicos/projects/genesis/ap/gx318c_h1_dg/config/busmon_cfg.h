/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __BUSMON_BOARD_CFG_H__
#define __BUSMON_BOARD_CFG_H__

#include <aic_auto_reg.h>
#include <aic_reg_base.h>
#include <irq_chip.h>
#include <busmon_inc.h>
#include <psram_mem_config.h>

static struct aic_busmon_board_cfg busmon_bd_cfg[] = {
    {
        .index = BM_MCU,
        .base = REG_BASE_AP_BUSMON_MCU,
        .glb_base = REG_MCU_GLB_BM_EB,
        .glb_bit = BIT_MCU_GLB_BM_EB,
        .mux_sel_base = REG_MCU_GLB_BM_CTRL,
        .mux_sel_eb = BIT_MCU_GLB_MUX_SEL_BM,
        .irq_no = IRQ_MCU_BM,
    },
    {
        .index = BM_CPUSYS,
        .base = REG_BASE_BUSMON_CPUSYS,
        .glb_base = REG_CPU_GLB_BM_EB,
        .glb_bit = BIT_CPU_GLB_BM_EB,
        .mux_sel_base = REG_CPU_GLB_BM_CTRL,
        .mux_sel_eb = BIT_CPU_GLB_MUX_SEL_BM,
        .irq_no = IRQ_CPU_BM,
    },
    {
        .index = BM_MEM_CP,
        .base = REG_BASE_BUSMON_MEMCTRL_CP,
        .glb_base = REG_MEM_CTRL_MEM_CTRL_DBG_BUS_CFG0,
        .glb_bit = BIT_MEM_CTRL_AXI_BUSMON_EN(0x1),
        .irq_no = IRQ_BM_CP,
    },
    {
        .index = BM_MEM_AP,
        .base = REG_BASE_BUSMON_MEMCTRL_MCU,
        .glb_base = REG_MEM_CTRL_MEM_CTRL_DBG_BUS_CFG0,
        .glb_bit = BIT_MEM_CTRL_AXI_BUSMON_EN(0x2),
        .irq_no = IRQ_BM_CPU,
    },
    {
        .index = BM_MEM_MODEM,
        .base = REG_BASE_BUSMON_MEMCTRL_MODEM,
        .glb_base = REG_MEM_CTRL_MEM_CTRL_DBG_BUS_CFG0,
        .glb_bit = BIT_MEM_CTRL_AXI_BUSMON_EN(0x4),
        .irq_no = IRQ_BM_ACC,
    },

};

#ifdef CONFIG_DRIVER_TRANS_MON
static struct aic_trans_mon_cfg trs_mon_cfg[] = {
    {REG_MCU_GLB_AXI_TRANS_MON_EN,      0xFFFF},
    {REG_CPU_GLB_AXI_TRANS_MON_EN,      0xFFFF},
    {REG_MEM_CTRL_MEM_CTRL_DBG_BUS_CFG0,0xFFFF},
    {REG_AON_GLB_AON_BUS_MON_EN,        0xFFFF}
};
#endif

struct gx_bm_def_mon {
    char *name;
    BUSMON_IP ip;
    u32 start;
    u32 end;
};

static struct gx_bm_def_mon gx_bm_mon[] = {
    {"MCU/Audio",   BM_MEM_AP,      CP_REGION_BASE,  CP_REGION_BASE + CP_REGION_SIZE - 1},
    {"CP",          BM_MEM_CP,      AP_REGION_BASE,  AP_REGION_BASE + AP_REGION_SIZE - 1},
    {"Modem",       BM_MEM_MODEM,   SHMEM_REGION_BASE,  PSRAM_REGION_BASE + PSRAM_REGION_SIZE - 1}
};

#endif /* __BUSMON_BOARD_CFG_H__ */

