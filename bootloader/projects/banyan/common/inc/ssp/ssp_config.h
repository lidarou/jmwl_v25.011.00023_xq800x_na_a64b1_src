/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines SSP configurations.
 */

#ifndef __SSP_CONFIG_H__
#define __SSP_CONFIG_H__

#include <aic_gpio_pin.h>
#include <reg_base.h>
#include <ckm_en_glb_mcu.h>
#include <cru_mcu.h>
#include <mcu_glb.h>
#include <aon_glb.h>
#include <top_glb.h>
#include <dma.h>
#include <pinlist.h>
#include <ssp.h>

#define SSP_CLK_DPLL0_96M       (96000000)
#define SSP_CLK_26M             (26000000)
#define SSP_CLK_MSRC0_48M       (48000000)
#define SSP_CLK_DPLL0_24P774M   (24774000)
#define SSP_CLK_DPLL1_24P576M   (245760004)

struct ssp_pin {
    u32 pin_reg;
    u32 func;
    GPIO_PIN_ID gpio_id;
    enum active_lvl active;
};

struct ssp_ctrl {
    char *name;
    u32 reg_base;
    u32 reset_reg;
    u32 enable_reg;
    u32 clk_reg;
    u32 src_clk;
    DMA_HWID dma_tx_id;
    DMA_HWID dma_rx_id;
    boolean dma_en;
    enum work_mode work_mode;
    enum bus_proto bus_proto;
    boolean auto_cs;
    u32 cs_num;
    struct ssp_pin *cs_pin;
};

static struct ssp_pin s_ssp_cs_pin[2] = {
    {REG_IOMUX_SSP_CS0, 3, GPIO_14, ACTIVE_LOW},
    {REG_IOMUX_SSP_CS1, 3, GPIO_16, ACTIVE_LOW}
};

static struct ssp_ctrl s_ssp_ctrl = {
    .name = "ssp0",
    .reg_base = REG_BASE_BLK_MCU_SSP0,
    .reset_reg = REG_CKM_EN_GLB_MCU_MCU_SSP0_SW_RST_GLB_CFG,
    .enable_reg = REG_CKM_EN_GLB_MCU_MCU_SSP0_EB,
    .clk_reg = REG_CRU_MCU_CLK_MCU_SSP0_CFG,
    .src_clk = SSP_CLK_DPLL0_96M,
    .dma_tx_id = DMA_REQ_MCU_SSP0_TX,
    .dma_rx_id = DMA_REQ_MCU_SSP0_RX,
    .dma_en = TRUE,
    .work_mode = MASTER_MODE,
    .bus_proto = PROTO_SPI,
    .auto_cs = FALSE,
    .cs_num = sizeof(s_ssp_cs_pin)/sizeof(struct ssp_pin),
    .cs_pin = s_ssp_cs_pin,
};

static inline struct ssp_ctrl *ssp_get_ctrl(void)
{
    return &s_ssp_ctrl;
}

#endif /* __SSP_CONFIG_H__ */
