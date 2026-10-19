/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file includes chip clock arrays file.
 */

#ifndef __AIC_CLK_H__
#define __AIC_CLK_H__

#include <aic_iot_clk.h>
#include "aic_clk_map.h"

static struct clk_gate clk_gate_array[] = {
    /* top en clocks */
    {
        .reg = 0x411f0008,
        .clk_id = CLK_AON_MTX_MAIN_EN,
        .index = 0,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_AON_MTX_M1_EN,
        .index = 1,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_AON_MTX_M2_EN,
        .index = 2,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_AON_AHB_MTX_S0_EN,
        .index = 3,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_TOP_APB_CRU_RF_EN,
        .index = 4,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_TOP_APB_CLK_EN_RF_EN,
        .index = 5,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_AON2CPU_MTX_EN,
        .index = 6,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_TOP2CPU_EIP2EFUSE_EN,
        .index = 7,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_AON_SYSTMR_EN,
        .index = 8,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_AON_TMR0_EN,
        .index = 9,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_AON_TMR1_EN,
        .index = 10,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_AON_TMR2_EN,
        .index = 11,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_AON_WDT0_EN,
        .index = 12,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_AON_WDT1_EN,
        .index = 13,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_AON_EIPU_EN,
        .index = 14,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_AON_EIPU2TOP_EN,
        .index = 15,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_AON_EIPU_SEC_EN,
        .index = 17,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_AON_TMR_SEC_EN,
        .index = 18,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_AON_WDT_SEC_EN,
        .index = 19,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_AON_GPD_EN,
        .index = 20,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_TOP_PWM0_EN,
        .index = 21,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_TOP_PWM1_EN,
        .index = 22,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_TOP_PWM2_EN,
        .index = 23,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_TOP_PWM3_EN,
        .index = 24,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_TOP_PWM4_EN,
        .index = 25,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_TOP_PWM5_EN,
        .index = 26,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_TOP_PWM6_EN,
        .index = 27,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_TOP_PWM7_EN,
        .index = 28,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_TOP_UART0_EN,
        .index = 29,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_TOP_UART1_EN,
        .index = 30,
    },
    {
        .reg = 0x411f0008,
        .clk_id = CLK_TOP_AON_UART_EN,
        .index = 31,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_AON_KPC_EN,
        .index = 0,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_TOP_MIPI_RFFE_EN,
        .index = 1,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_TOP_IIS0_EN,
        .index = 2,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_TOP_IIS1_EN,
        .index = 2,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_PSRAMC_2X_EN,
        .index = 6,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_PSRAMC_EN,
        .index = 7,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_PSRAMPHY_AT_SPEED_EN,
        .index = 8,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_AXI_CPU2MEM_EN,
        .index = 10,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_REF_NORFLASH_EN,
        .index = 13,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_REF_NORFLASH_DIV2_EN,
        .index = 14,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_AON_PWR_EN,
        .index = 15,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_AON_PMU_26M_EN,
        .index = 16,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_TOP_SDIOPHY_EN,
        .index = 19,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_AON_DAP_EN,
        .index = 20,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_AUX0_EN,
        .index = 21,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_AUX1_EN,
        .index = 22,
    },
    {
        .reg = 0x411f001c,
        .clk_id = CLK_AUX2_EN,
        .index = 23,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_ZJTAG_CTRL_TCK_EN,
        .index = 24,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_I_THERM_EN,
        .index = 25,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_OCMC_EN,
        .index = 26,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_26M_PSRAMC,
        .index = 27,
    },
    {
        .reg = 0x411f000c,
        .clk_id = CLK_32K_PMU_EN,
        .index = 28,
    },
    /* mcu en clocks */
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_CMSTAR_EN,
        .index = 0,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_CMSTAR_DAP_EN,
        .index = 1,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_MTX_S5_EN,
        .index = 2,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_MTX_M1_EN,
        .index = 3,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_MTX_M0_EN,
        .index = 4,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_MTX_MAIN_EN,
        .index = 5,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_MTX_M2_EN,
        .index = 6,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_AXI_DMA_EN,
        .index = 7,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_MTX_M8_EN,
        .index = 8,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_MTX_M9_EN,
        .index = 9,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_AXI_PPP_E_EN,
        .index = 10,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_AXI_PPP_D_EN,
        .index = 11,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_MTX_M4_EN,
        .index = 12,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_AXI_USB_EN,
        .index = 13,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_MTX_S4_EN,
        .index = 14,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_MTX_M6_EN,
        .index = 15,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_AHB_LCDC_EN,
        .index = 16,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_MTX_M7_EN,
        .index = 17,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_AHB_CSI_EN,
        .index = 18,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_MTX_S1_EN,
        .index = 19,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_AHB_GLB_EN,
        .index = 20,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_AHB_LPC_EN,
        .index = 21,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_AHB_ROMCODE_EN,
        .index = 22,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_AHB_EIP_EN,
        .index = 23,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_MTX_S3_EN,
        .index = 24,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MERGE_MTX_M4_EN,
        .index = 25,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_MTX_S2_EN,
        .index = 26,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_AHB2APB_EN,
        .index = 27,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_APB_I2C0_EN,
        .index = 28,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_APB_I2C1_EN,
        .index = 29,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_APB_UART0_EN,
        .index = 30,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_APB_UART1_EN,
        .index = 31,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_APB_UART2_EN,
        .index = 0,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_APB_TMR_EN,
        .index = 1,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_APB_SDIOH_EN,
        .index = 2,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_APB_SSP0_EN,
        .index = 3,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_APB_SSP1_EN,
        .index = 4,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_APB_PPP_D_EN,
        .index = 5,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_APB_PPP_E_EN,
        .index = 6,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_APB_LCDC_EN,
        .index = 7,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_APB_CSI_EN,
        .index = 8,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_CP_APB_PCDUMP_EN,
        .index = 9,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_UART0_EN,
        .index = 10,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_UART1_EN,
        .index = 11,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_UART2_EN,
        .index = 12,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_SSP0_EN,
        .index = 13,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_SSP1_EN,
        .index = 14,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_LCDC_EN,
        .index = 15,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_EIP_EN,
        .index = 16,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_MTX_M3_EN,
        .index = 17,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_EIP_CTR_EN,
        .index = 18,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_SDIOH_EN,
        .index = 19,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_TMR_EN,
        .index = 20,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_PPP_D_EN,
        .index = 21,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_PPP_E_EN,
        .index = 22,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MERGE_MTX_S3_EN,
        .index = 23,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_MTX_M10_EN,
        .index = 24,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MERGE_MTX_MAIN_MEM_EN,
        .index = 25,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_MTX_S0_EN,
        .index = 26,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_AUDIO_MTX_S0_EN,
        .index = 27,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MERGE_MTX_M0_EN,
        .index = 28,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MERGE_MTX_M1_EN,
        .index = 29,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MERGE_MTX_S0_EN,
        .index = 30,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MERGE_MTX_MAIN_PER_EN,
        .index = 31,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_MERGE_MTX_S5_EN,
        .index = 0,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_AUDIO_MTX_M3_EN,
        .index = 1,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_MERGE_MTX_S1_EN,
        .index = 2,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_MERGE_MTX_M5_EN,
        .index = 3,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_MERGE_MTX_MAIN_DBG_EN,
        .index = 4,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_MCU_SDPHY_REG_PCLK_EN,
        .index = 5,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_MCU_MTX_M5_EN,
        .index = 6,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_MERGE_MTX_S2_EN,
        .index = 7,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_CRU_MCU_RF_EN,
        .index = 8,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_CRU_AUDIO_RF_EN,
        .index = 9,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_CRU_CP_RF_EN,
        .index = 10,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_CKM_EN_MCU_RF_EN,
        .index = 11,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_CKM_EN_AUDIO_RF_EN,
        .index = 12,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_CKM_EN_CP_RF_EN,
        .index = 13,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_CPU_AHB_LPC_EN,
        .index = 14,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_CPU_AHB_LZMA_EN,
        .index = 15,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_CPU_GLB_EN,
        .index = 16,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_MCU_UTMI_EN,
        .index = 17,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_MCU_USB_PTEST_EN,
        .index = 18,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_MCU_PTEST_EN,
        .index = 19,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_EIP2EFUSE_EN,
        .index = 20,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_MCU_LZMA_EN,
        .index = 21,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_MERGE_MTX_M6_EN,
        .index = 22,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_CPU_DAP_EN,
        .index = 23,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_CPU_ZJTAG_TCK_EN,
        .index = 24,
    },
    /* Cp gate clocks*/
    {
        .reg = 0x42040008,
        .clk_id = CLK_BLK_CP_CORE_EN,
        .index = 0,
    },
    /* dsp gate clock */
    {
        .reg = 0x42020008,
        .clk_id = CLK_BLK_DSP_CORE_EN,
        .index = 0,
    },
    /* pll power up need some delay, bit_0 stands for need wait time */
    {
        .reg = 0x400C002d,
        .clk_id = CLK_UPLL_EN,
        .index = 3,
    },
};

static struct clk_pll_gate clk_pll_gate_array[] = {
    {
        .gate.reg = 0x400C002c,
        .gate.clk_id = CLK_UPLL_EN,
        .gate.index = 3,
        .enable_cnt = 0,
        .delay_time = 100,
    },
    {
        .gate.reg = 0x400d0154,
        .gate.clk_id = CLK_UPLL_24M_EN,
        .gate.index = 0,
        .enable_cnt = 0,
        .delay_time = 0,
    },
    {
        .gate.reg = 0x400c002c,
        .gate.clk_id = CLK_APLL_EN,
        .gate.index = 1,
        .enable_cnt = 0,
        .delay_time = 100,
    }
};

const struct clk_parent clk_ocmc_parents[] = {
    {
        .index = 0,
        .rate = 200000000,
    },
    {
        .index = 1,
        .rate = 125000000,
    },
    {
        .index = 2,
        .rate = 26000000,
    },
};

const struct clk_parent clk_aon_kpc_parents[] = {
    {
        .index = 0,
        .rate = 26000000,
    },
    {
        .index = 1,
        .rate = 32000,
    },
};

const struct clk_parent clk_blk_cp_core_parents[] = {
    {
        .index = 0,
        .rate = 614400000,
    },
    {
        .index = 1,
        .rate = 368640000,
    },
    {
        .index = 2,
        .rate = 307200000,
    },
    {
        .index = 3,
        .rate = 26000000,
    },
};

const struct clk_parent clk_blk_dsp_core_parents[] = {
    {
        .index = 0,
        .rate = 307200000,
    },
    {
        .index = 1,
        .rate = 200000000,
    },
    {
        .index = 2,
        .rate = 26000000,
    },
};

static struct clk_mux clk_mux_array[] = {
    {
        .reg = 0x411e0048,
        .clk_id = CLK_OCMC,

        .parents = clk_ocmc_parents,
        .parents_num = 3,
        .cur_parent_index = 2,

        .mux_width = 2,
        .mux_shift = 0,
    },

    {
        .reg = 0x42010024,
        .clk_id = CLK_MCU_EIP_CTR,

        .parents = clk_aon_kpc_parents,
        .parents_num = 2,
        .cur_parent_index = 0,

        .mux_width = 1,
        .mux_shift = 0,
    },
};

static struct clk_div clk_div_array[] = {
    {
        .reg = 0x411e0028,
        .clk_id = CLK_AON_PMU_26M,

        .prate = 26000000,

        .div_width = 4,
        .div_shift = 0,
    },

    {
        .reg = 0x411e002c,
        .clk_id = CLK_AON_SIM0,

        .prate = 32000,

        .div_width = 4,
        .div_shift = 0,
    },

    {
        .reg = 0x411e0044,
        .clk_id = CLK_ZJTAG_CTRL_TCK,

        .prate = 26000000,

        .div_width = 4,
        .div_shift = 0,
    },

    {
        .reg = 0x411e004c,
        .clk_id = CLK_26M_PSRAMC,

        .prate = 26000000,
        .div_width = 4,
        .div_shift = 0,
    },
};

const struct clk_parent clk_aon_mtx_main_parents[] = {
    {
        .index = 0,
        .rate = 125000000,
    },
    {
        .index = 1,
        .rate = 26000000,
    },
    {
        .index = 2,
        .rate = 32000,
    },
};

const struct clk_parent clk_aon_gpd_parents[] = {
    {
        .index = 0,
        .rate = 100000000,
    },
    {
        .index = 1,
        .rate = 26000000,
    },
    {
        .index = 2,
        .rate = 32000,
    },
};

const struct clk_parent clk_top_iis0_parents[] = {
    {
        .index = 0,
        .rate = 26000000,
    },
    {
        .index = 1,
        .rate = 24576000,
    },
    {
        .index = 2,
        .rate = 192000000,
    },
};

const struct clk_parent clk_pasramc_parents[] = {
    {
        .index = 0,
        .rate = 500000000,
    },
    {
        .index = 1,
        .rate = 396000000,
    },
    {
        .index = 2,
        .rate = 333000000,
    },
    {
        .index = 3,
        .rate = 26000000,
    },
};

const struct clk_parent clk_ref_norflash_parents[] = {
    {
        .index = 0,
        .rate = 264000000,
    },
    {
        .index = 1,
        .rate = 250000000,
    },
    {
        .index = 2,
        .rate = 200000000,
    },
    {
        .index = 3,
        .rate = 26000000,
    },
};

const struct clk_parent clk_top_sdiophy_parents[] = {
    {
        .index = 0,
        .rate = 200000000,
    },
    {
        .index = 1,
        .rate = 125000000,
    },
    {
        .index = 2,
        .rate = 26000000,
    },
};

const struct clk_parent clk_aon_dap_parents[] = {
    {
        .index = 0,
        .rate = 26000000,
    },
    {
        .index = 1,
        .rate = 32000
    },
};

const struct clk_parent clk_aux_parents[] = {
    {
        .index = 0,
        .rate = 245760000,
    },
    {
        .index = 1,
        .rate = 200000000,
    },
    {
        .index = 2,
        .rate = 26000000,
    },
    {
        .index = 3,
        .rate = 24576000,
    },
    {
        .index = 4,
        .rate = 24000000,
    },
    {
        .index = 5,
        .rate = 19200000,
    },
    {
        .index = 6,
        .rate = 32000,
    },
};

const struct clk_parent clk_aux2_parents[] = {
    {
        .index = 0,
        .rate = 200000000,
    },
    {
        .index = 1,
        .rate = 26000000,
    },
    {
        .index = 2,
        .rate = 24576000,
    },
    {
        .index = 3,
        .rate = 24000000,
    },
    {
        .index = 4,
        .rate = 19200000,
    },
    {
        .index = 5,
        .rate = 32000,
    },
};

const struct clk_parent clk_mcu_cmstar_parents[] = {
    {
        .index = 0,
        .rate = 500000000,
    },
    {
        .index = 1,
        .rate = 307200000,
    },
    {
        .index = 2,
        .rate = 200000000,
    },
    {
        .index = 3,
        .rate = 26000000,
    },
};

const struct clk_parent clk_mcu_ahb2apb_parents[] = {
    {
        .index = 0,
        .rate = 125000000,
    },
    {
        .index = 1,
        .rate = 100000000,
    },
    {
        .index = 2,
        .rate = 26000000,
    },
};

const struct clk_parent clk_mcu_uart_parents[] = {
    {
        .index = 0,
        .rate = 100000000,
    },
    {
        .index = 1,
        .rate = 26000000,
    },
};

const struct clk_parent clk_mcu_ssp_parents[] = {
    {
        .index = 0,
        .rate = 100000000,
    },
    {
        .index = 1,
        .rate = 26000000,
    },
    {
        .index = 2,
        .rate = 24576000,
    },
};

const struct clk_parent clk_mcu_tmr_parents[] = {
    {
        .index = 0,
        .rate = 26000000,
    },
    {
        .index = 1,
        .rate = 2000000,
    },
};

const struct clk_parent clk_mcu_ppp_parents[] = {
    {
        .index = 0,
        .rate = 125000000,
    },
    {
        .index = 1,
        .rate = 26000000,
    },
};

const struct clk_parent clk_mcu_ptest_parents[] = {
    {
        .index = 0,
        .rate = 100000000,
    },
    {
        .index = 1,
        .rate = 26000000,
    },
};

const struct clk_parent clk_mcu_lzma_parents[] = {
    {
        .index = 0,
        .rate = 307200000,
    },
    {
        .index = 1,
        .rate = 200000000,
    },
    {
        .index = 2,
        .rate = 26000000,
    },
};

static struct clk_comp clk_comp_array[] = {
    /* TOP clocks */
    {
        .reg = 0x411e0000,
        .clk_id = CLK_AON_MTX_MAIN,

        .parents = clk_aon_mtx_main_parents,
        .parents_num = 3,
        .cur_parent_index = 1,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },

    {
        .reg = 0x411e0004,
        .clk_id = CLK_AON_GPD,

        .parents = clk_aon_gpd_parents,
        .parents_num = 3,
        .cur_parent_index = 1,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 2,
        .div_shift = 4,
    },

    {
        .reg = 0x411e0008,
        .clk_id = CLK_TOP_PWM0,

        .parents = clk_aon_gpd_parents,
        .parents_num = 3,
        .cur_parent_index = 1,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },

    {
        .reg = 0x411e000c,
        .clk_id = CLK_AON_KPC,

        .parents = clk_aon_kpc_parents,
        .parents_num = 2,
        .cur_parent_index = 0,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 3,
        .div_shift = 1,
    },

    {
        .reg = 0x411e0010,
        .clk_id = CLK_TOP_MIPI_RFFE,

        .parents = clk_aon_gpd_parents,
        .parents_num = 3,
        .cur_parent_index = 1,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 2,
        .div_shift = 3,
    },

    {
        .reg = 0x411e0014,
        .clk_id = CLK_TOP_IIS0,

        .parents = clk_top_iis0_parents,
        .parents_num = 3,
        .cur_parent_index = 0,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },

    {
        .reg = 0x411e001c,
        .clk_id = CLK_PSRAMC_2X,

        .parents = clk_pasramc_parents,
        .parents_num = 4,
        .cur_parent_index = 3,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },

    {
        .reg = 0x411e0020,
        .clk_id = CLK_REF_NORFLASH,

        .parents = clk_ref_norflash_parents,
        .parents_num = 4,
        .cur_parent_index = 3,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },

    {
        .reg = 0x411e0030,
        .clk_id = CLK_TOP_SDIOPHY,

        .parents = clk_top_sdiophy_parents,
        .parents_num = 3,
        .cur_parent_index = 2,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },

    {
        .reg = 0x411e0034,
        .clk_id = CLK_AON_DAP,

        .parents = clk_aon_dap_parents,
        .parents_num = 2,
        .cur_parent_index = 0,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },

    {
        .reg = 0x411e0038,
        .clk_id = CLK_AUX0,

        .parents = clk_aux_parents,
        .parents_num = 7,
        .cur_parent_index = 2,

        .mux_width = 3,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 3,
    },

    {
        .reg = 0x411e003c,
        .clk_id = CLK_AUX1,

        .parents = clk_aux_parents,
        .parents_num = 7,
        .cur_parent_index = 2,

        .mux_width = 3,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 3,
    },

    {
        .reg = 0x411e0040,
        .clk_id = CLK_AUX2,

        .parents = clk_aux2_parents,
        .parents_num = 7,
        .cur_parent_index = 2,

        .mux_width = 3,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 3,
    },

    /* mcu clocks */
    {
        .reg = 0x42010000,
        .clk_id = CLK_MCU_CMSTAR,

        .parents = clk_mcu_cmstar_parents,
        .parents_num = 4,
        .cur_parent_index = 3,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },

    {
        .reg = 0x42010004,
        .clk_id = CLK_MCU_AHB2APB,

        .parents = clk_mcu_ahb2apb_parents,
        .parents_num = 3,
        .cur_parent_index = 2,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },

    {
        .reg = 0x42010008,
        .clk_id = CLK_MCU_UART0,

        .parents = clk_mcu_uart_parents,
        .parents_num = 2,
        .cur_parent_index = 1,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },

    {
        .reg = 0x4201000c,
        .clk_id = CLK_MCU_UART1,

        .parents = clk_mcu_uart_parents,
        .parents_num = 2,
        .cur_parent_index = 1,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },

    {
        .reg = 0x42010010,
        .clk_id = CLK_MCU_UART2,

        .parents = clk_mcu_uart_parents,
        .parents_num = 2,
        .cur_parent_index = 1,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },

    {
        .reg = 0x42010014,
        .clk_id = CLK_MCU_SSP0,

        .parents = clk_mcu_ssp_parents,
        .parents_num = 3,
        .cur_parent_index = 1,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },

    {
        .reg = 0x42010018,
        .clk_id = CLK_MCU_SSP1,

        .parents = clk_mcu_ssp_parents,
        .parents_num = 3,
        .cur_parent_index = 1,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },

    {
        .reg = 0x4201001c,
        .clk_id = CLK_MCU_LCDC,

        .parents = clk_mcu_uart_parents,
        .parents_num = 2,
        .cur_parent_index = 1,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },

    {
        .reg = 0x42010020,
        .clk_id = CLK_MCU_EIP,

        .parents = clk_top_sdiophy_parents,
        .parents_num = 3,
        .cur_parent_index = 2,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },

    {
        .reg = 0x42010028,
        .clk_id = CLK_MCU_TMR,

        .parents = clk_mcu_tmr_parents,
        .parents_num = 2,
        .cur_parent_index = 0,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },

    {
        .reg = 0x4201002c,
        .clk_id = CLK_MCU_PPP_D,

        .parents = clk_mcu_ppp_parents,
        .parents_num = 2,
        .cur_parent_index = 0,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },

    {
        .reg = 0x42010030,
        .clk_id = CLK_MCU_PPP_E,

        .parents = clk_mcu_ppp_parents,
        .parents_num = 2,
        .cur_parent_index = 0,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },

    {
        .reg = 0x42010034,
        .clk_id = CLK_MCU_MERGE_MTX_S2,

        .parents = clk_mcu_ppp_parents,
        .parents_num = 2,
        .cur_parent_index = 0,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },

    {
        .reg = 0x42010038,
        .clk_id = CLK_MCU_USB_PTEST,

        .parents = clk_mcu_ppp_parents,
        .parents_num = 2,
        .cur_parent_index = 0,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },

    {
        .reg = 0x4201003c,
        .clk_id = CLK_MCU_PTEST,

        .parents = clk_mcu_ptest_parents,
        .parents_num = 2,
        .cur_parent_index = 0,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },

    {
        .reg = 0x42010040,
        .clk_id = CLK_MCU_LZMA,

        .parents = clk_mcu_lzma_parents,
        .parents_num = 3,
        .cur_parent_index = 0,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },

    /* cp clocks */
    {
        .reg = 0x42050000,
        .clk_id = CLK_BLK_CP_CORE,
        .parents = clk_blk_cp_core_parents,
        .parents_num = 4,
        .cur_parent_index = 3,
        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },
    /* dsp clocks */
    {
        .reg = 0x42030000,
        .clk_id = CLK_BLK_DSP_CORE,
        .parents = clk_blk_dsp_core_parents,
        .parents_num = 3,
        .cur_parent_index = 2,
        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },
};

#endif /* __AIC_CLK_H__ */

