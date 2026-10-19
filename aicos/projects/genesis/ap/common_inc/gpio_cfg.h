/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __GPIO_BOARD_CFG_H__
#define __GPIO_BOARD_CFG_H__

#include <aic_auto_reg.h>
#include <aic_reg_base.h>
#include <aic_gpio_pin.h>
#include <irq.h>
#include <pinlist.h>

#define GPIO_PULSE_DEF_INDEX        GPIO_TOP_17
#define GPIO_AON_CNT                16
#define GPIO_PD_CNT                 32
/* clear clamp_EN/Val slp_EN bits */
#define GPIO_PIN_CLR_MASK           0xF6000


struct aic_gpio_board_cfg {
    u32 index;
    u32 base;
    u32 glb_base;
    u32 glb_bit;
    //u32 bp_oe_en;
    //u32 bp_oe_out;
    //u32 bp_ie_en;
    //u32 bp_ie_in;
    u32 irq_no;
    u32 pin_cnt;
};

struct aic_gpio_aon_pin_map {
    u32 func_base;
    u32 func_val;
    u32 multi_func_base;
    u32 multi_func_val;
    u32 ctrl_base;
    u32 multi_ctrl_base;
};

struct aic_gpio_pd_pin_map {
    u32 func_base;
    u32 ctrl_base;
};

static struct aic_gpio_aon_pin_map gpio_aon_map[16] = {
    {REG_IOMUX_CAM_MCLK,    6, REG_IOMUX_UART0_TXD, 3,  REG_PG0_PAD_CTRL_CAM_MCLK,      REG_PG10_PAD_CTRL_UART0_TXD},//gpio0
    {REG_IOMUX_CAM_RST,     0, 0,                   0,  REG_PG0_PAD_CTRL_CAM_RST,       0                          },//gpio1
    {REG_IOMUX_CAM_PWDN,    0, 0,                   0,  REG_PG0_PAD_CTRL_CAM_PWDN,      0                          },//gpio2
    {REG_IOMUX_CAM_SPI_D0,  6, REG_IOMUX_UART0_RTS, 3,  REG_PG0_PAD_CTRL_CAM_SPI_D0,    REG_PG10_PAD_CTRL_UART0_RTS},//gpio3
    {REG_IOMUX_CAM_SPI_D1,  6, REG_IOMUX_KEYIN4,    3,  REG_PG0_PAD_CTRL_CAM_SPI_D1,    REG_PG12_PAD_CTRL_KEYIN4},   //gpio4
    {REG_IOMUX_CAM_SPI_CLK, 6, REG_IOMUX_KEYIN5,    3,  REG_PG0_PAD_CTRL_CAM_SPI_CLK,   REG_PG12_PAD_CTRL_KEYIN5},   //gpio5
    {REG_IOMUX_SIM1_CLK,    6, REG_IOMUX_KEYOUT4,   3,  REG_PG8_PAD_CTRL_SIM1_CLK,      REG_PG12_PAD_CTRL_KEYOUT4},  //gpio6
    {REG_IOMUX_SSP1_CLK,    6, REG_IOMUX_KEYOUT5,   3,  REG_PG10_PAD_CTRL_SSP1_CLK,     REG_PG12_PAD_CTRL_KEYOUT5},  //gpio7
    {REG_IOMUX_SSP1_CS0,    6, REG_IOMUX_KEYIN2,    3,  REG_PG10_PAD_CTRL_SSP1_CS0,     REG_PG12_PAD_CTRL_KEYIN2},   //gpio8
    {REG_IOMUX_SSP1_MISO,   6, REG_IOMUX_KEYIN3,    3,  REG_PG10_PAD_CTRL_SSP1_MISO,    REG_PG12_PAD_CTRL_KEYIN3},   //gpio9
    {REG_IOMUX_SSP1_MOSI,   6, REG_IOMUX_KEYOUT0,   3,  REG_PG10_PAD_CTRL_SSP1_MOSI,    REG_PG12_PAD_CTRL_KEYOUT0},  //gpio10
    {REG_IOMUX_SSP1_CS1,    6, REG_IOMUX_KEYOUT1,   3,  REG_PG10_PAD_CTRL_SSP1_CS1,     REG_PG12_PAD_CTRL_KEYOUT1},  //gpio11
    {REG_IOMUX_SSP0_CLK,    6, REG_IOMUX_KEYOUT2,   3,  REG_PG11_PAD_CTRL_SSP0_CLK,     REG_PG12_PAD_CTRL_KEYOUT2},  //gpio12
    {REG_IOMUX_SSP0_CS0,    6, REG_IOMUX_KEYOUT3,   3,  REG_PG11_PAD_CTRL_SSP0_CS0,     REG_PG12_PAD_CTRL_KEYOUT3},  //gpio13
    {REG_IOMUX_UART0_RXD,   3, REG_IOMUX_SSP0_MISO, 6,  REG_PG10_PAD_CTRL_UART0_RXD,    REG_PG11_PAD_CTRL_SSP0_MISO},//gpio14
    {REG_IOMUX_UART0_CTS,   3, REG_IOMUX_SSP0_MOSI, 6,  REG_PG10_PAD_CTRL_UART0_CTS,    REG_PG11_PAD_CTRL_SSP0_MOSI},//gpio15
};

static struct aic_gpio_pd_pin_map gpio_pd_map[32] = {
    {REG_IOMUX_RFCTRL5,     REG_PG13_PAD_CTRL_RFCTRL5},     //gpio16
    {REG_IOMUX_DAP_TDI,     REG_PG3_PAD_CTRL_DAP_TDI},      //gpio17
    {REG_IOMUX_DAP_TRST,    REG_PG3_PAD_CTRL_DAP_TRST},     //gpio18
    {REG_IOMUX_IIC1_SDA,    REG_PG0_PAD_CTRL_IIC1_SDA},     //gpio19
    {REG_IOMUX_IIC1_SCL,    REG_PG0_PAD_CTRL_IIC1_SCL},     //gpio20
    {REG_IOMUX_UART1_RXD,   REG_PG4_PAD_CTRL_UART1_RXD},    //gpio21
    {REG_IOMUX_UART3_RXD,   REG_PG4_PAD_CTRL_UART3_RXD},    //gpio22
    {REG_IOMUX_SPI_LCD_SDI, REG_PG5_PAD_CTRL_SPI_LCD_SDI},  //gpio23
    {REG_IOMUX_SPI_LCD_SDO, REG_PG5_PAD_CTRL_SPI_LCD_SDO},  //gpio24
    {REG_IOMUX_SPI_LCD_DCX, REG_PG5_PAD_CTRL_SPI_LCD_DCX},  //gpio25
    {REG_IOMUX_SPI_LCD_CS,  REG_PG5_PAD_CTRL_SPI_LCD_CS},   //gpio26
    {REG_IOMUX_SPI_LCD_CLK, REG_PG5_PAD_CTRL_SPI_LCD_CLK},  //gpio27
    {REG_IOMUX_LCD_FMARK,   REG_PG5_PAD_CTRL_LCD_FMARK},    //gpio28
    {REG_IOMUX_LCD_RSTN,    REG_PG5_PAD_CTRL_LCD_RSTN},     //gpio29
    {REG_IOMUX_SDMMC_CLK,   REG_PG6_PAD_CTRL_SDMMC_CLK},    //gpio30
    {REG_IOMUX_SDMMC_CMD,   REG_PG6_PAD_CTRL_SDMMC_CMD},    //gpio31
    {REG_IOMUX_SDMMC_RST,   REG_PG7_PAD_CTRL_SDMMC_RST},    //gpio32
    {REG_IOMUX_SDMMC_D0,    REG_PG6_PAD_CTRL_SDMMC_D0},     //gpio33
    {REG_IOMUX_SDMMC_D1,    REG_PG6_PAD_CTRL_SDMMC_D1},     //gpio34
    {REG_IOMUX_SDMMC_D2,    REG_PG6_PAD_CTRL_SDMMC_D2},     //gpio35
    {REG_IOMUX_SDMMC_D3,    REG_PG6_PAD_CTRL_SDMMC_D3},     //gpio36
    {REG_IOMUX_SDMMC_D4,    REG_PG7_PAD_CTRL_SDMMC_D4},     //gpio37
    {REG_IOMUX_SDMMC_D5,    REG_PG7_PAD_CTRL_SDMMC_D5},     //gpio38
    {REG_IOMUX_SDMMC_D6,    REG_PG7_PAD_CTRL_SDMMC_D6},     //gpio39
    {REG_IOMUX_SDMMC_D7,    REG_PG7_PAD_CTRL_SDMMC_D7},     //gpio40
    {REG_IOMUX_SIM1_RST,    REG_PG8_PAD_CTRL_SIM1_RST},     //gpio41
    {REG_IOMUX_SIM1_DAT,    REG_PG8_PAD_CTRL_SIM1_DAT},     //gpio42
    {REG_IOMUX_IIS0_CLK,    REG_PG11_PAD_CTRL_IIS0_CLK},    //gpio43
    {REG_IOMUX_IIS0_LRCLK,  REG_PG11_PAD_CTRL_IIS0_LRCLK},  //gpio44
    {REG_IOMUX_IIS0_DI,     REG_PG11_PAD_CTRL_IIS0_DI},     //gpio45
    {REG_IOMUX_IIS0_DO,     REG_PG11_PAD_CTRL_IIS0_DO},     //gpio46
    {REG_IOMUX_IIS0_MCLK,   REG_PG11_PAD_CTRL_IIS0_MCLK},   //gpio47
};

__attribute__ ((section(".idle_data")))
static struct aic_gpio_board_cfg gpio_bd_cfg[GPIO_IP_MAX] = {
    {
        .index = GPIO_AON,
        .base = REG_BASE_AON_GPIO,
        .glb_base = REG_AON_GLB_GPIO_EB,
        .glb_bit = BIT_AON_GLB_GPIO_EB,
        //.bp_oe_en = REG_AON_GLB_N_GPIO_BYPASS_OE_AON,
        //.bp_oe_out = REG_AON_GLB_GPIO_BYPASS_OUT_AON,
        //.bp_ie_en = REG_AON_GLB_N_GPIO_BYPASS_IE_AON,
        //.bp_ie_in = REG_AON_GLB_GPIO_BYPASS_IN_AON,
        .irq_no = IRQ_AON_GPIO,
        .pin_cnt = 16,
    },
    {
        .index = GPIO_TOP,
        .base = REG_BASE_TOP_GPIO,
        .glb_base = REG_TOP_GLB_GPIO_EB,
        .glb_bit = BIT_TOP_GLB_GPIO_EB,
        //.bp_oe_en = REG_TOP_GLB_N_GPIO_BYPASS_OE_TOP,
        //.bp_oe_out = REG_TOP_GLB_GPIO_BYPASS_OUT_TOP,
        //.bp_ie_en = REG_TOP_GLB_N_GPIO_BYPASS_IE_TOP,
        //.bp_ie_in = REG_TOP_GLB_GPIO_BYPASS_IN_TOP,
        .irq_no = IRQ_TOP_GPIO,
        .pin_cnt = 32,
    },
};

#endif /* __GPIO_BOARD_CFG_H__ */

