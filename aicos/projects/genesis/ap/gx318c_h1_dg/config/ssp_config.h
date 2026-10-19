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
#include <cru_mcu.h>
#include <mcu_glb.h>
#include <aon_glb.h>
#include <top_glb.h>
#include <dma.h>
#include <pinlist.h>
#include <ssp.h>

#define SSP_CLK_GPLL_100M   (100000000)
#define SSP_CLK_26M         (26000000)
#define SSP_CLK_APLL_24M    (24576000)

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
    IRQn_Type int_no;
    enum work_mode work_mode;
    enum bus_proto bus_proto;
    u32 cs_num;
    struct ssp_pin *cs_pin;
};

struct ssp_pin s_ssp0_cs_pin[1] = {
    {REG_IOMUX_SSP0_CS0, 6, GPIO_AON_13, ACTIVE_LOW}
};

struct ssp_pin s_ssp1_cs_pin[2] = {
    {REG_IOMUX_SSP1_CS0, 6, GPIO_AON_8, ACTIVE_LOW},
    {REG_IOMUX_SSP1_CS1, 6, GPIO_AON_11, ACTIVE_LOW},
};

static struct ssp_ctrl s_ssp_ctrl[2] = {
    {
        .name = "ssp0",
        .reg_base = REG_BASE_AP_SSPC0,
        .reset_reg = REG_MCU_GLB_SW_RST_GLB_CLK_MCU_SSP0,
        .enable_reg = REG_MCU_GLB_SSP_0_EB,
        .clk_reg = REG_CRU_MCU_CLK_MCU_SSP0_CFG,
        .src_clk = SSP_CLK_GPLL_100M,
        .dma_tx_id = DMA_REQ_MCU_SSP0_TX,
        .dma_rx_id = DMA_REQ_MCU_SSP0_RX,
        .dma_en = FALSE,
        .int_no = IRQ_SSP0,
        .work_mode = MASTER_MODE,
        .bus_proto = PROTO_SPI,
        .cs_num = sizeof(s_ssp0_cs_pin)/sizeof(struct ssp_pin),
        .cs_pin = s_ssp0_cs_pin,
    },
    {
        .name = "ssp1",
        .reg_base = REG_BASE_AP_SSPC1,
        .reset_reg = REG_MCU_GLB_SW_RST_GLB_CLK_MCU_SSP1,
        .enable_reg = REG_MCU_GLB_SSP_1_EB,
        .clk_reg = REG_CRU_MCU_CLK_MCU_SSP1_CFG,
        .src_clk = SSP_CLK_GPLL_100M,
        .dma_tx_id = DMA_REQ_MCU_SSP1_TX,
        .dma_rx_id = DMA_REQ_MCU_SSP1_RX,
        .dma_en = TRUE,
        .int_no = IRQ_SSP1,
        .work_mode = MASTER_MODE,
        .bus_proto = PROTO_SPI,
        .cs_num = sizeof(s_ssp1_cs_pin)/sizeof(struct ssp_pin),
        .cs_pin = s_ssp1_cs_pin,
    }
};

static inline struct ssp_ctrl *ssp_get_ctrl(u32 id)
{
    return &s_ssp_ctrl[id];
}

static inline u32 ssp_get_ctrl_num(void)
{
    return sizeof(s_ssp_ctrl)/sizeof(struct ssp_ctrl);
}

#endif /* __SSP_CONFIG_H__ */
