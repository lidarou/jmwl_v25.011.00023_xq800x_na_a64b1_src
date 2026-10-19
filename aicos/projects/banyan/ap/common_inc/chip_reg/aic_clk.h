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
#if 0
    /* top en clocks */
    {
        .reg = 0x410e0000,
        .clk_id = CLK_TOP_SBUS_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0004,
        .clk_id = CLK_TOP_I2CC_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0008,
        .clk_id = CLK_TOP_I2C_PMIC_EB,
        .index = 0,
    },
    {
        .reg = 0x410e000c,
        .clk_id = CLK_TOP_THMC_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0010,
        .clk_id = CLK_TOP_UART_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0014,
        .clk_id = CLK_TOP_IIS_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0018,
        .clk_id = CLK_TOP_UART_TX_EB,
        .index = 0,
    },
    {
        .reg = 0x410e001c,
        .clk_id = CLK_TOP_SYSTMR_RF_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0020,
        .clk_id = CLK_TOP_MAILBOX_EB,
        .index = 8,
    },
    {
        .reg = 0x410e0024,
        .clk_id = CLK_TOP_SPINLOCK_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0028,
        .clk_id = CLK_TOP_PLL_RF_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0030,
        .clk_id = CLK_TOP_PMRO_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0034,
        .clk_id = CLK_TOP_EFUSE_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0038,
        .clk_id = CLK_TOP_PWM0_EB,
        .index = 0,
    },
    {
        .reg = 0x410e003c,
        .clk_id = CLK_TOP_PWM1_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0040,
        .clk_id = CLK_TOP_GPIO_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0044,
        .clk_id = CLK_TOP_WDT_MCU_EB,
        .index = 0,
    },
    {
        .reg = 0x410e004c,
        .clk_id = CLK_TOP_TRNG_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0050,
        .clk_id = CLK_PSRAMC_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0054,
        .clk_id = CLK_MEM_MAIN_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0058,
        .clk_id = CLK_AXI_CP2MEM_EB,
        .index = 0,
    },
    {
        .reg = 0x410e005c,
        .clk_id = CLK_AXI_CPU2MEM_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0060,
        .clk_id = CLK_AXI_PERF_M0_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0064,
        .clk_id = CLK_AXI_PERF_M1_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0074,
        .clk_id = CLK_AXI_PERF_M2_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0078,
        .clk_id = CLK_NORFLASH_EB,
        .index = 0,
    },
    {
        .reg = 0x410e007c,
        .clk_id = CLK_AUX0_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0080,
        .clk_id = CLK_AUX1_EB,
        .index = 0,
    },
    {
        .reg = 0x410e0084,
        .clk_id = CLK_OCMC_EB,
        .index = 0,
    },
    {
        .reg = 0x410e00a0,
        .clk_id = CLK_CAM_MCLK_EB,
        .index = 0,
    },
    {
        .reg = 0x410e00a4,
        .clk_id = CLK_USB_REF_EB,
        .index = 0,
    },
    {
        .reg = 0x410e00a8,
        .clk_id = CLK_CAL_26M_EB,
        .index = 0,
    },
    {
        .reg = 0x410e00ac,
        .clk_id = CLK_TOP2CPU_DAP_EB,
        .index = 0,
    },
    {
        .reg = 0x410e00b0,
        .clk_id = CLK_RCO_MOD_EB,
        .index = 0,
    },
    {
        .reg = 0x410e00b4,
        .clk_id = CLK_LPUART_PD_EB,
        .index = 0,
    },
    {
        .reg = 0x40020000,
        .clk_id = CLK_AON_GLB_EB,
        .index = 0,
    },
    {
        .reg = 0x40020004,
        .clk_id = CLK_AON_IPU_CP_EB,
        .index = 0,
    },
    {
        .reg = 0x40020008,
        .clk_id = CLK_AON_SYSTMR_CNT_EB,
        .index = 0,
    },
    {
        .reg = 0x40020014,
        .clk_id = CLK_AON_EIPU_MCU_EB,
        .index = 0,
    },
    {
        .reg = 0x40020020,
        .clk_id = CLK_CMSTAR_DAP_EB,
        .index = 0,
    },
    {
        .reg = 0x42000000,
        .clk_id = CLK_CMSTAR_EB,
        .index = 0,
    },
    {
        .reg = 0x42000004,
        .clk_id = CLK_CMSTAR_DAP_EB,
        .index = 0,
    },
    {
        .reg = 0x42000008,
        .clk_id = CLK_MCU_MTX_MAIN0_EB,
        .index = 0,
    },
    {
        .reg = 0x4200000c,
        .clk_id = CLK_MCU_LZMA_EB,
        .index = 0,
    },
    {
        .reg = 0x42000010,
        .clk_id = CLK_MCU_MTX_S3_EB,
        .index = 0,
    },
    {
        .reg = 0x42000014,
        .clk_id = CLK_MCU_MTB_EB,
        .index = 0,
    },
    {
        .reg = 0x42000018,
        .clk_id = CLK_ROMCODE_EB,
        .index = 0,
    },
    {
        .reg = 0x4200001c,
        .clk_id = CLK_MCU_MTX_MAIN1_EB,
        .index = 0,
    },
    {
        .reg = 0x42000020,
        .clk_id = CLK_MCU_DMA_EB,
        .index = 0,
    },
    {
        .reg = 0x42000024,
        .clk_id = CLK_MCU_PPPE_EB,
        .index = 0,
    },
    {
        .reg = 0x42000028,
        .clk_id = CLK_MCU_PPPD_EB,
        .index = 0,
    },
    {
        .reg = 0x4200002c,
        .clk_id = CLK_MCU_USB_EB,
        .index = 0,
    },
    {
        .reg = 0x42000030,
        .clk_id = CLK_MCU_LCDC_EB,
        .index = 0,
    },
    {
        .reg = 0x42000034,
        .clk_id = CLK_MCU_CSI_EB,
        .index = 0,
    },
    {
        .reg = 0x42000038,
        .clk_id = CLK_MCU_MTX_S2_EB,
        .index = 0,
    },
    {
        .reg = 0x4200003c,
        .clk_id = CLK_MCU_AHB2AHB_EB,
        .index = 0,
    },
    {
        .reg = 0x42000040,
        .clk_id = CLK_MCU_AHB2APB_EB,
        .index = 0,
    },
    {
        .reg = 0x42000044,
        .clk_id = CLK_MCU_I2C0_EB,
        .index = 0,
    },
    {
        .reg = 0x42000048,
        .clk_id = CLK_MCU_I2C1_EB,
        .index = 0,
    },
    {
        .reg = 0x4200004c,
        .clk_id = CLK_MCU_UART0_EB,
        .index = 0,
    },
    {
        .reg = 0x42000050,
        .clk_id = CLK_MCU_UART1_EB,
        .index = 0,
    },
    {
        .reg = 0x42000054,
        .clk_id = CLK_MCU_SSP0_EB,
        .index = 0,
    },
    {
        .reg = 0x42000058,
        .clk_id = CLK_MCU_CAN_EB,
        .index = 0,
    },
    {
        .reg = 0x4200005c,
        .clk_id = CLK_MCU_TMR_EB,
        .index = 0,
    },
    {
        .reg = 0x42000060,
        .clk_id = CLK_CR5_PCDUMP_EB,
        .index = 0,
    },
    {
        .reg = 0x42000064,
        .clk_id = CLK_MCU_CAN_TMR_EB,
        .index = 0,
    },
    {
        .reg = 0x42000068,
        .clk_id = CLK_CPU_MTX_EB,
        .index = 0,
    },
    {
        .reg = 0x4200006c,
        .clk_id = CLK_MCU_MTX_S0_EB,
        .index = 0,
    },
    {
        .reg = 0x42000070,
        .clk_id = CLK_MCU_PTEST_EB,
        .index = 0,
    },
    {
        .reg = 0x42000074,
        .clk_id = CLK_CPU_DAP_TPI_EB,
        .index = 0,
    },
    {
        .reg = 0x42000078,
        .clk_id = CLK_CPU_DAP_TPI_EB,
        .index = 0,
    },
    {
        .reg = 0x400e0030,
        .clk_id = CLK_ND_EIPU_EB,
        .index = 0,
    },
    {
        .reg = 0x400e0034,
        .clk_id = CLK_ND_UART_RX_EB,
        .index = 0,
    },
    {
        .reg = 0x400e0038,
        .clk_id = CLK_NON_ND_UART_RX_EB,
        .index = 0,
    },
    {
        .reg = 0x400e003c,
        .clk_id = CLK_ND_TMR_MCU_EB,
        .index = 0,
    },
    {
        .reg = 0x400e0040,
        .clk_id = CLK_PMU_32K_EB,
        .index = 0,
    },
    {
        .reg = 0x400e0044,
        .clk_id = CLK_AON_SBUS_EB,
        .index = 0,
    },
    {
        .reg = 0x400e0048,
        .clk_id = CLK_ND_PWR_EB,
        .index = 0,
    },
#endif
};

static struct clk_pll_gate clk_pll_gate_array[] = {
    {
        .gate.reg = 0x400a0500,
        .gate.clk_id = CLK_DPLL1_EN,
        .gate.index = 0,
        .enable_cnt = 0,
        .delay_time = 100,
    },
    {
        .gate.reg = 0x410e00a0,
        .gate.clk_id = CLK_CAM_MCLK_EB,
        .gate.index = 0,
        .enable_cnt = 0,
        .delay_time = 0,
    },
};

const struct clk_parent clk_top_iis_parents[] = {
    {
        .index = 0,
        .rate = 26000000,
    },
    {
        .index = 1,
        .rate = 24774000,
    },
    {
        .index = 2,
        .rate = 24576000,
    },
    {
        .index = 3,
        .rate = 19200000,
    },
    {
        .index = 4,
        .rate = 12288000,
    },
    {
        .index = 5,
        .rate = 12288000,
    },
};

const struct clk_parent clk_ocmc_parents[] = {
    {
        .index = 0,
        .rate = 192000000,
    },
    {
        .index = 1,
        .rate = 128000000,
    },
    {
        .index = 2,
        .rate = 26000000,
    },
};

const struct clk_parent clk_cam_mclk_parents[] = {
    {
        .index = 0,
        .rate = 26000000,
    },
    {
        .index = 1,
        .rate = 24000000,
    },
};

const struct clk_parent clk_usb_ref_parents[] = {
    {
        .index = 0,
        .rate = 26000000,
    },
    {
        .index = 1,
        .rate = 24000000,
    },
    {
        .index = 2,
        .rate = 19200000,
    },
};

const struct clk_parent clk_rco_parents[] = {
    {
        .index = 0,
        .rate = 48000000,
    },
    {
        .index = 1,
        .rate = 406800,
    },
    {
        .index = 2,
        .rate = 32768,
    },
};

const struct clk_parent clk_top_lpuart_pd_parents[] = {
    {
        .index = 0,
        .rate = 26000000,
    },
    {
        .index = 1,
        .rate = 460800,
    },
    {
        .index = 2,
        .rate = 32768,
    },
};

static struct clk_mux clk_mux_array[] = {
    {
        .reg = 0x410d0020,
        .clk_id = CLK_TOP_IIS_CFG,

        .parents = clk_top_iis_parents,
        .parents_num = 6,
        .cur_parent_index = 0,

        .mux_width = 3,
        .mux_shift = 0,
    },

    {
        .reg = 0x410d003c,
        .clk_id = CLK_OCMC_CFG,

        .parents = clk_ocmc_parents,
        .parents_num = 3,
        .cur_parent_index = 2,

        .mux_width = 2,
        .mux_shift = 0,
    },
    {
        .reg = 0x410d0044,
        .clk_id = CLK_CAM_MCLK_CFG,

        .parents = clk_cam_mclk_parents,
        .parents_num = 2,
        .cur_parent_index = 0,

        .mux_width = 1,
        .mux_shift = 0,
    },
    {
        .reg = 0x410d0048,
        .clk_id = CLK_USB_REF_CFG,

        .parents = clk_usb_ref_parents,
        .parents_num = 3,
        .cur_parent_index = 1,

        .mux_width = 2,
        .mux_shift = 0,
    },
    {
        .reg = 0x410d0050,
        .clk_id = CLK_RCO_CFG,

        .parents = clk_rco_parents,
        .parents_num = 3,
        .cur_parent_index = 2,

        .mux_width = 2,
        .mux_shift = 0,
    },
    {
        .reg = 0x410d0054,
        .clk_id = CLK_TOP_LPUART_PD_CFG,

        .parents = clk_top_lpuart_pd_parents,
        .parents_num = 3,
        .cur_parent_index = 0,

        .mux_width = 2,
        .mux_shift = 0,
    },
    {
        .reg = 0x400e0000,
        .clk_id = CLK_ND_UART_RX,

        .parents = clk_top_lpuart_pd_parents,
        .parents_num = 3,
        .cur_parent_index = 0,

        .mux_width = 2,
        .mux_shift = 0,
    },
};

static struct clk_div clk_div_array[] = {
    {
        .reg = 0,
        .clk_id = 0xFF,
        .prate = 0,
        .div_width = 0,
        .div_shift = 0,
    },

/*    {
        .reg = 0x410d0014,
        .clk_id = CLK_TOP_WDT_MCU_CFG,

        .prate = 32768,

        .div_width = 4,
        .div_shift = 0,
    },

    {
        .reg = 0x410d0024,
        .clk_id = CLK_TOP_TRNG_CFG,

        .prate = 26000000,

        .div_width = 4,
        .div_shift = 0,
    },

    {
        .reg = 0x42010004,
        .clk_id = CLK_MCU_CAN_CFG,

        .prate = 26000000,

        .div_width = 4,
        .div_shift = 0,
    },*/
};

/*const struct clk_parent clk_top_apb_sbus_parents[] = {
    {
        .index = 0,
        .rate = 128000000,
    },
    {
        .index = 1,
        .rate = 26000000,
    },
    {
        .index = 2,
        .rate = 32768,
    },
};

const struct clk_parent clk_top_efuse_parents[] = {
    {
        .index = 0,
        .rate = 26000000,
    },
    {
        .index = 1,
        .rate = 32768,
    },
};

const struct clk_parent clk_top_pwm0_parents[] = {
    {
        .index = 0,
        .rate = 96000000,
    },
    {
        .index = 1,
        .rate = 26000000,
    },
    {
        .index = 2,
        .rate = 32768,
    },
};*/

const struct clk_parent clk_top_uart_parents[] = {
    {
        .index = 0,
        .rate = 96000000,
    },
    {
        .index = 1,
        .rate = 26000000,
    },
    {
        .index = 2,
        .rate = 460800,
    },
    {
        .index = 3,
        .rate = 32768,
    },
};

const struct clk_parent clk_psramc_2x_parents[] = {
    {
        .index = 0,
        .rate = 475136000,
    },
    {
        .index = 1,
        .rate = 384000000,
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

const struct clk_parent clk_ref_norflash_parents[] = {
    {
        .index = 0,
        .rate = 256000000,
    },
    {
        .index = 1,
        .rate = 192000000,
    },
    {
        .index = 2,
        .rate = 26000000,
    },
};

/*const struct clk_parent clk_aux0_parents[] = {
    {
        .index = 0,
        .rate = 184320000,
    },
    {
        .index = 1,
        .rate = 192000000,
    },
    {
        .index = 2,
        .rate = 122880000,
    },
    {
        .index = 3,
        .rate = 26000000,
    },
    {
        .index = 4,
        .rate = 24774000,
    },
    {
        .index = 5,
        .rate = 24576000,
    },
    {
        .index = 6,
        .rate = 12288000,
    },
    {
        .index = 7,
        .rate = 32768,
    },
};

const struct clk_parent clk_top2cpu_dap_parents[] = {
    {
        .index = 0,
        .rate = 26000000,
    },
    {
        .index = 1,
        .rate = 32768,
    },
};*/

const struct clk_parent clk_mcu_cmstar_parents[] = {
    {
        .index = 0,
        .rate = 384000000,
    },
    {
        .index = 1,
        .rate = 356352000,
    },
    {
        .index = 2,
        .rate = 307200000,
    },
    {
        .index = 3,
        .rate = 256000000,
    },
    {
        .index = 4,
        .rate = 512000000,
    },
    {
        .index = 5,
        .rate = 26000000,
    },
};

const struct clk_parent clk_mcu_uart0_parents[] = {
    {
        .index = 0,
        .rate = 96000000,
    },
    {
        .index = 1,
        .rate = 26000000,
    },
    {
        .index = 2,
        .rate = 460800,
    },
};

const struct clk_parent clk_mcu_ssp0_parents[] = {
    {
        .index = 0,
        .rate = 96000000,
    },
    {
        .index = 1,
        .rate = 26000000,
    },
    {
        .index = 2,
        .rate = 26000000,
    },
    {
        .index = 3,
        .rate = 24774000,
    },
    {
        .index = 4,
        .rate = 24576000,
    },
};

const struct clk_parent clk_mcu_lcdc_parents[] = {
    {
        .index = 0,
        .rate = 96000000,
    },
    {
        .index = 1,
        .rate = 26000000,
    },
};

/*const struct clk_parent clk_mcu_can_tmr_parents[] = {
    {
        .index = 0,
        .rate = 26000000,
    },
    {
        .index = 1,
        .rate = 2000000,
    },
};

const struct clk_parent clk_mcu_pppd_parents[] = {
    {
        .index = 0,
        .rate = 128000000,
    },
    {
        .index = 1,
        .rate = 26000000,
    },
};*/


static struct clk_comp clk_comp_array[] = {
    /* TOP clocks */
    /*{
        .reg = 0x410d0000,
        .clk_id = CLK_TOP_APB_SBUS_CFG,

        .parents = clk_top_apb_sbus_parents,
        .parents_num = 3,
        .cur_parent_index = 1,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },
    {
        .reg = 0x410d0004,
        .clk_id = CLK_TOP_EFUSE_CFG,

        .parents = clk_top_efuse_parents,
        .parents_num = 2,
        .cur_parent_index = 0,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },
    {
        .reg = 0x410d0008,
        .clk_id = CLK_TOP_PWM0_CFG,

        .parents = clk_top_pwm0_parents,
        .parents_num = 3,
        .cur_parent_index = 1,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },
    {
        .reg = 0x410d000c,
        .clk_id = CLK_TOP_PWM1_CFG,

        .parents = clk_top_pwm0_parents,
        .parents_num = 3,
        .cur_parent_index = 1,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },
    {
        .reg = 0x410d0010,
        .clk_id = CLK_TOP_GPIO_CFG,

        .parents = clk_top_pwm0_parents,
        .parents_num = 3,
        .cur_parent_index = 1,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },*/
    {
        .reg = 0x410d001c,
        .clk_id = CLK_TOP_UART_CFG,

        .parents = clk_top_uart_parents,
        .parents_num = 4,
        .cur_parent_index = 1,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },
    {
        .reg = 0x410d0028,
        .clk_id = CLK_PSRAMC_2X_CFG,

        .parents = clk_psramc_2x_parents,
        .parents_num = 4,
        .cur_parent_index = 3,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },
    {
        .reg = 0x410d0030,
        .clk_id = CLK_REF_NORFLASH_CFG,

        .parents = clk_ref_norflash_parents,
        .parents_num = 3,
        .cur_parent_index = 2,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },
    /*{
        .reg = 0x410d0034,
        .clk_id = CLK_AUX0_CFG,

        .parents = clk_aux0_parents,
        .parents_num = 8,
        .cur_parent_index = 7,

        .mux_width = 3,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 3,
    },
    {
        .reg = 0x410d0038,
        .clk_id = CLK_AUX1_CFG,

        .parents = clk_aux0_parents,
        .parents_num = 8,
        .cur_parent_index = 7,

        .mux_width = 3,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 3,
    },

    {
        .reg = 0x410d004c,
        .clk_id = CLK_TOP2CPU_DAP_CFG,

        .parents = clk_top2cpu_dap_parents,
        .parents_num = 2,
        .cur_parent_index = 0,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },
    {
        .reg = 0x40000000,
        .clk_id = CLK_AON_DAP_CFG,

        .parents = clk_top2cpu_dap_parents,
        .parents_num = 2,
        .cur_parent_index = 0,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },*/
    {
        .reg = 0x42010000,
        .clk_id = CLK_MCU_CMSTAR_CFG,

        .parents = clk_mcu_cmstar_parents,
        .parents_num = 6,
        .cur_parent_index = 5,

        .mux_width = 3,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 3,
    },
    {
        .reg = 0x42010008,
        .clk_id = CLK_MCU_UART0_CFG,

        .parents = clk_mcu_uart0_parents,
        .parents_num = 3,
        .cur_parent_index = 1,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },
    {
        .reg = 0x4201000c,
        .clk_id = CLK_MCU_UART1_CFG,

        .parents = clk_mcu_uart0_parents,
        .parents_num = 3,
        .cur_parent_index = 1,

        .mux_width = 2,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 2,
    },

    {
        .reg = 0x42010010,
        .clk_id = CLK_MCU_SSP0_CFG,

        .parents = clk_mcu_ssp0_parents,
        .parents_num = 5,
        .cur_parent_index = 1,

        .mux_width = 3,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 3,
    },
    {
        .reg = 0x42010014,
        .clk_id = CLK_MCU_LCDC_CFG,

        .parents = clk_mcu_lcdc_parents,
        .parents_num = 2,
        .cur_parent_index = 1,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },
#if 0
    {
        .reg = 0x42010018,
        .clk_id = CLK_MCU_CAN_TMR_CFG,

        .parents = clk_mcu_can_tmr_parents,
        .parents_num = 2,
        .cur_parent_index = 0,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },
    {
        .reg = 0x42010020,
        .clk_id = CLK_MCU_PPPD_CFG,

        .parents = clk_mcu_pppd_parents,
        .parents_num = 2,
        .cur_parent_index = 1,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },
    {
        .reg = 0x42010024,
        .clk_id = CLK_MCU_PPPE_CFG,

        .parents = clk_mcu_pppd_parents,
        .parents_num = 2,
        .cur_parent_index = 1,

        .mux_width = 1,
        .mux_shift = 0,
        .div_width = 4,
        .div_shift = 1,
    },
#endif
};

#endif /* __AIC_CLK_H__ */

