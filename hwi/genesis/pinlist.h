/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Pinlist.
*/

#ifndef __PINLIST_H__
#define __PINLIST_H__

#include <aic_bitmap.h>
#include <reg_base.h>

/* Cross Bar */
#define REG_IOMUX_IO_CROSSBAR_SEL_UART      (REG_BASE_AON_IO_MUX + 0x10)
#define REG_IOMUX_IO_CROSSBAR_SEL_IIS       (REG_BASE_AON_IO_MUX + 0x14)
#define REG_IOMUX_IO_CROSSBAR_SEL_SIM       (REG_BASE_AON_IO_MUX + 0x18)
#define REG_IOMUX_IO_CROSSBAR_SEL_IIC       (REG_BASE_AON_IO_MUX + 0x1c)
#define REG_IOMUX_IO_CROSSBAR_SEL_RFFE      (REG_BASE_AON_IO_MUX + 0x20)

/* PAD Power CTRL */
#define REG_PG5_PAD_CTRL_PWR_PAD_CTRL       (REG_BASE_TOP_PG5_PAD_CTRL + 0x0)
#define REG_PG6_PAD_CTRL_PWR_PAD_CTRL       (REG_BASE_TOP_PG6_PAD_CTRL + 0x0)
#define REG_PG7_PAD_CTRL_PWR_PAD_CTRL       (REG_BASE_TOP_PG7_PAD_CTRL + 0x0)
#define REG_PG8_PAD_CTRL_PWR_PAD_CTRL       (REG_BASE_TOP_PG8_PAD_CTRL + 0x0)
#define REG_PG9_PAD_CTRL_PWR_PAD_CTRL       (REG_BASE_TOP_PG9_PAD_CTRL + 0x0)
#define REG_PG10_PAD_CTRL_PWR_PAD_CTRL      (REG_BASE_TOP_PG10_PAD_CTRL + 0x0)
#define REG_PG12_PAD_CTRL_PWR_PAD_CTRL      (REG_BASE_TOP_PG12_PAD_CTRL + 0x0)

/* io mux */
#define REG_IOMUX_IIC1_SDA                  (REG_BASE_AON_IO_MUX + 0x24)
#define REG_IOMUX_IIC1_SCL                  (REG_BASE_AON_IO_MUX + 0x28)
#define REG_IOMUX_CAM_MCLK                  (REG_BASE_AON_IO_MUX + 0x2c)
#define REG_IOMUX_CAM_RST                   (REG_BASE_AON_IO_MUX + 0x30)
#define REG_IOMUX_CAM_PWDN                  (REG_BASE_AON_IO_MUX + 0x34)
#define REG_IOMUX_CAM_SPI_D0                (REG_BASE_AON_IO_MUX + 0x38)
#define REG_IOMUX_CAM_SPI_D1                (REG_BASE_AON_IO_MUX + 0x3c)
#define REG_IOMUX_CAM_SPI_CLK               (REG_BASE_AON_IO_MUX + 0x40)
#define REG_IOMUX_IIC0_SDA                  (REG_BASE_AON_IO_MUX + 0x44)
#define REG_IOMUX_IIC0_SCL                  (REG_BASE_AON_IO_MUX + 0x48)
#define REG_IOMUX_RST_REQ                   (REG_BASE_AON_IO_MUX + 0x4c)
#define REG_IOMUX_PMIC_INT_N                (REG_BASE_AON_IO_MUX + 0x50)
#define REG_IOMUX_EXT_EN0                   (REG_BASE_AON_IO_MUX + 0x54)
#define REG_IOMUX_EXT_EN1                   (REG_BASE_AON_IO_MUX + 0x58)
#define REG_IOMUX_CHIP_SLEEP                (REG_BASE_AON_IO_MUX + 0x5c)
#define REG_IOMUX_DAP_TCK                   (REG_BASE_AON_IO_MUX + 0x60)
#define REG_IOMUX_DAP_TMS                   (REG_BASE_AON_IO_MUX + 0x64)
#define REG_IOMUX_DAP_TDO                   (REG_BASE_AON_IO_MUX + 0x68)
#define REG_IOMUX_DAP_TDI                   (REG_BASE_AON_IO_MUX + 0x6c)
#define REG_IOMUX_DAP_TRST                  (REG_BASE_AON_IO_MUX + 0x70)
#define REG_IOMUX_SPI_NF_SIP_CS0            (REG_BASE_AON_IO_MUX + 0x74)
#define REG_IOMUX_SPI_NF_SIP_D1             (REG_BASE_AON_IO_MUX + 0x78)
#define REG_IOMUX_SPI_NF_SIP_D2             (REG_BASE_AON_IO_MUX + 0x7c)
#define REG_IOMUX_SPI_NF_SIP_D0             (REG_BASE_AON_IO_MUX + 0x80)
#define REG_IOMUX_SPI_NF_SIP_SCLK           (REG_BASE_AON_IO_MUX + 0x84)
#define REG_IOMUX_SPI_NF_SIP_D3             (REG_BASE_AON_IO_MUX + 0x88)
#define REG_IOMUX_UART1_TXD                 (REG_BASE_AON_IO_MUX + 0x8c)
#define REG_IOMUX_UART1_RXD                 (REG_BASE_AON_IO_MUX + 0x90)
#define REG_IOMUX_UART3_TXD                 (REG_BASE_AON_IO_MUX + 0x94)
#define REG_IOMUX_UART3_RXD                 (REG_BASE_AON_IO_MUX + 0x98)
#define REG_IOMUX_SPI_LCD_SDI               (REG_BASE_AON_IO_MUX + 0x9c)
#define REG_IOMUX_SPI_LCD_SDO               (REG_BASE_AON_IO_MUX + 0xa0)
#define REG_IOMUX_SPI_LCD_DCX               (REG_BASE_AON_IO_MUX + 0xa4)
#define REG_IOMUX_SPI_LCD_CS                (REG_BASE_AON_IO_MUX + 0xa8)
#define REG_IOMUX_SPI_LCD_CLK               (REG_BASE_AON_IO_MUX + 0xac)
#define REG_IOMUX_LCD_FMARK                 (REG_BASE_AON_IO_MUX + 0xb0)
#define REG_IOMUX_LCD_RSTN                  (REG_BASE_AON_IO_MUX + 0xb4)
#define REG_IOMUX_SDMMC_D0                  (REG_BASE_AON_IO_MUX + 0xb8)
#define REG_IOMUX_SDMMC_D1                  (REG_BASE_AON_IO_MUX + 0xbc)
#define REG_IOMUX_SDMMC_D2                  (REG_BASE_AON_IO_MUX + 0xc0)
#define REG_IOMUX_SDMMC_D3                  (REG_BASE_AON_IO_MUX + 0xc4)
#define REG_IOMUX_SDMMC_CLK                 (REG_BASE_AON_IO_MUX + 0xc8)
#define REG_IOMUX_SDMMC_CMD                 (REG_BASE_AON_IO_MUX + 0xcc)
#define REG_IOMUX_SDMMC_RST                 (REG_BASE_AON_IO_MUX + 0xd0)
#define REG_IOMUX_SDMMC_D4                  (REG_BASE_AON_IO_MUX + 0xd4)
#define REG_IOMUX_SDMMC_D5                  (REG_BASE_AON_IO_MUX + 0xd8)
#define REG_IOMUX_SDMMC_D6                  (REG_BASE_AON_IO_MUX + 0xdc)
#define REG_IOMUX_SDMMC_D7                  (REG_BASE_AON_IO_MUX + 0xe0)
#define REG_IOMUX_SDMMC_REB                 (REG_BASE_AON_IO_MUX + 0xe4)
#define REG_IOMUX_SIM1_RST                  (REG_BASE_AON_IO_MUX + 0xe8)
#define REG_IOMUX_SIM1_DAT                  (REG_BASE_AON_IO_MUX + 0xec)
#define REG_IOMUX_SIM1_CLK                  (REG_BASE_AON_IO_MUX + 0xf0)
#define REG_IOMUX_SIM0_RST                  (REG_BASE_AON_IO_MUX + 0xf4)
#define REG_IOMUX_SIM0_DAT                  (REG_BASE_AON_IO_MUX + 0xf8)
#define REG_IOMUX_SIM0_CLK                  (REG_BASE_AON_IO_MUX + 0xfc)
#define REG_IOMUX_UART0_TXD                 (REG_BASE_AON_IO_MUX + 0x100)
#define REG_IOMUX_UART0_RXD                 (REG_BASE_AON_IO_MUX + 0x104)
#define REG_IOMUX_UART0_CTS                 (REG_BASE_AON_IO_MUX + 0x108)
#define REG_IOMUX_UART0_RTS                 (REG_BASE_AON_IO_MUX + 0x10c)
#define REG_IOMUX_SSP1_CS1                  (REG_BASE_AON_IO_MUX + 0x110)
#define REG_IOMUX_SSP1_CLK                  (REG_BASE_AON_IO_MUX + 0x114)
#define REG_IOMUX_SSP1_CS0                  (REG_BASE_AON_IO_MUX + 0x118)
#define REG_IOMUX_SSP1_MISO                 (REG_BASE_AON_IO_MUX + 0x11c)
#define REG_IOMUX_SSP1_MOSI                 (REG_BASE_AON_IO_MUX + 0x120)
#define REG_IOMUX_IIS0_MCLK                 (REG_BASE_AON_IO_MUX + 0x124)
#define REG_IOMUX_IIS0_CLK                  (REG_BASE_AON_IO_MUX + 0x128)
#define REG_IOMUX_IIS0_LRCLK                (REG_BASE_AON_IO_MUX + 0x12c)
#define REG_IOMUX_IIS0_DI                   (REG_BASE_AON_IO_MUX + 0x130)
#define REG_IOMUX_IIS0_DO                   (REG_BASE_AON_IO_MUX + 0x134)
#define REG_IOMUX_SSP0_CLK                  (REG_BASE_AON_IO_MUX + 0x138)
#define REG_IOMUX_SSP0_CS0                  (REG_BASE_AON_IO_MUX + 0x13c)
#define REG_IOMUX_SSP0_MISO                 (REG_BASE_AON_IO_MUX + 0x140)
#define REG_IOMUX_SSP0_MOSI                 (REG_BASE_AON_IO_MUX + 0x144)
#define REG_IOMUX_KEYIN4                    (REG_BASE_AON_IO_MUX + 0x148)
#define REG_IOMUX_KEYIN5                    (REG_BASE_AON_IO_MUX + 0x14c)
#define REG_IOMUX_KEYOUT4                   (REG_BASE_AON_IO_MUX + 0x150)
#define REG_IOMUX_KEYOUT5                   (REG_BASE_AON_IO_MUX + 0x154)
#define REG_IOMUX_KEYIN0                    (REG_BASE_AON_IO_MUX + 0x158)
#define REG_IOMUX_KEYIN1                    (REG_BASE_AON_IO_MUX + 0x15c)
#define REG_IOMUX_KEYIN2                    (REG_BASE_AON_IO_MUX + 0x160)
#define REG_IOMUX_KEYIN3                    (REG_BASE_AON_IO_MUX + 0x164)
#define REG_IOMUX_KEYOUT0                   (REG_BASE_AON_IO_MUX + 0x168)
#define REG_IOMUX_KEYOUT1                   (REG_BASE_AON_IO_MUX + 0x16c)
#define REG_IOMUX_KEYOUT2                   (REG_BASE_AON_IO_MUX + 0x170)
#define REG_IOMUX_KEYOUT3                   (REG_BASE_AON_IO_MUX + 0x174)
#define REG_IOMUX_RFCTRL0                   (REG_BASE_AON_IO_MUX + 0x178)
#define REG_IOMUX_RFCTRL1                   (REG_BASE_AON_IO_MUX + 0x17c)
#define REG_IOMUX_RFCTRL2                   (REG_BASE_AON_IO_MUX + 0x180)
#define REG_IOMUX_RFCTRL3                   (REG_BASE_AON_IO_MUX + 0x184)
#define REG_IOMUX_RFCTRL4                   (REG_BASE_AON_IO_MUX + 0x188)
#define REG_IOMUX_RFCTRL5                   (REG_BASE_AON_IO_MUX + 0x18c)
#define REG_IOMUX_RFCTRL6                   (REG_BASE_AON_IO_MUX + 0x190)
#define REG_IOMUX_RFCTRL7                   (REG_BASE_AON_IO_MUX + 0x194)

/* PAD CTRL */
/* pin group0 */
#define REG_PG0_PAD_CTRL_IIC1_SDA           (REG_BASE_TOP_PG0_PAD_CTRL + 0x10)
#define REG_PG0_PAD_CTRL_IIC1_SCL           (REG_BASE_TOP_PG0_PAD_CTRL + 0x14)
#define REG_PG0_PAD_CTRL_CAM_MCLK           (REG_BASE_TOP_PG0_PAD_CTRL + 0x18)
#define REG_PG0_PAD_CTRL_CAM_RST            (REG_BASE_TOP_PG0_PAD_CTRL + 0x1c)
#define REG_PG0_PAD_CTRL_CAM_PWDN           (REG_BASE_TOP_PG0_PAD_CTRL + 0x20)
#define REG_PG0_PAD_CTRL_CAM_SPI_D0         (REG_BASE_TOP_PG0_PAD_CTRL + 0x24)
#define REG_PG0_PAD_CTRL_CAM_SPI_D1         (REG_BASE_TOP_PG0_PAD_CTRL + 0x28)
#define REG_PG0_PAD_CTRL_CAM_SPI_CLK        (REG_BASE_TOP_PG0_PAD_CTRL + 0x2c)

/* pin group1 */
#define REG_PG1_PAD_CTRL_PSRAM_DQS          (REG_BASE_TOP_PG1_PAD_CTRL + 0x10)
#define REG_PG1_PAD_CTRL_PSRAM_RWDS         (REG_BASE_TOP_PG1_PAD_CTRL + 0x14)
#define REG_PG1_PAD_CTRL_PSRAM_DQ7          (REG_BASE_TOP_PG1_PAD_CTRL + 0x18)
#define REG_PG1_PAD_CTRL_PSRAM_DQ6          (REG_BASE_TOP_PG1_PAD_CTRL + 0x1c)
#define REG_PG1_PAD_CTRL_PSRAM_DQ5          (REG_BASE_TOP_PG1_PAD_CTRL + 0x20)
#define REG_PG1_PAD_CTRL_PSRAM_DQ4          (REG_BASE_TOP_PG1_PAD_CTRL + 0x24)
#define REG_PG1_PAD_CTRL_PSRAM_CLK_P        (REG_BASE_TOP_PG1_PAD_CTRL + 0x28)
#define REG_PG1_PAD_CTRL_PSRAM_CLK_N        (REG_BASE_TOP_PG1_PAD_CTRL + 0x2c)
#define REG_PG1_PAD_CTRL_PSRAM_CE           (REG_BASE_TOP_PG1_PAD_CTRL + 0x30)
#define REG_PG1_PAD_CTRL_PSRAM_DQ3          (REG_BASE_TOP_PG1_PAD_CTRL + 0x34)
#define REG_PG1_PAD_CTRL_PSRAM_DQ2          (REG_BASE_TOP_PG1_PAD_CTRL + 0x38)
#define REG_PG1_PAD_CTRL_PSRAM_DQ1          (REG_BASE_TOP_PG1_PAD_CTRL + 0x3c)
#define REG_PG1_PAD_CTRL_PSRAM_DQ0          (REG_BASE_TOP_PG1_PAD_CTRL + 0x40)
#define REG_PG1_PAD_CTRL_PSRAM_DM           (REG_BASE_TOP_PG1_PAD_CTRL + 0x44)
#define REG_PG1_PAD_CTRL_PSRAM_RESETB       (REG_BASE_TOP_PG1_PAD_CTRL + 0x48)

/* pin group2 */
#define REG_PG2_PAD_CTRL_IIC0_SDA           (REG_BASE_TOP_PG2_PAD_CTRL + 0x10)
#define REG_PG2_PAD_CTRL_IIC0_SCL           (REG_BASE_TOP_PG2_PAD_CTRL + 0x14)
#define REG_PG2_PAD_CTRL_BB_32K_IN          (REG_BASE_TOP_PG2_PAD_CTRL + 0x18)
#define REG_PG2_PAD_CTRL_RST_REQ            (REG_BASE_TOP_PG2_PAD_CTRL + 0x1c)
#define REG_PG2_PAD_CTRL_PMIC_INT_N         (REG_BASE_TOP_PG2_PAD_CTRL + 0x20)
#define REG_PG2_PAD_CTRL_EXT_EN0            (REG_BASE_TOP_PG2_PAD_CTRL + 0x24)
#define REG_PG2_PAD_CTRL_EXT_EN1            (REG_BASE_TOP_PG2_PAD_CTRL + 0x28)
#define REG_PG2_PAD_CTRL_CHIP_SLEEP         (REG_BASE_TOP_PG2_PAD_CTRL + 0x2c)

/* pin group3 */
#define REG_PG3_PAD_CTRL_DAP_TCK            (REG_BASE_TOP_PG3_PAD_CTRL + 0x10)
#define REG_PG3_PAD_CTRL_DAP_TMS            (REG_BASE_TOP_PG3_PAD_CTRL + 0x14)
#define REG_PG3_PAD_CTRL_DAP_TDO            (REG_BASE_TOP_PG3_PAD_CTRL + 0x18)
#define REG_PG3_PAD_CTRL_DAP_TDI            (REG_BASE_TOP_PG3_PAD_CTRL + 0x1c)
#define REG_PG3_PAD_CTRL_DAP_TRST           (REG_BASE_TOP_PG3_PAD_CTRL + 0x20)

/* pin group4 */
#define REG_PG4_PAD_CTRL_SPI_NF_SIP_CS0     (REG_BASE_TOP_PG4_PAD_CTRL + 0x10)
#define REG_PG4_PAD_CTRL_SPI_NF_SIP_D1      (REG_BASE_TOP_PG4_PAD_CTRL + 0x14)
#define REG_PG4_PAD_CTRL_SPI_NF_SIP_D2      (REG_BASE_TOP_PG4_PAD_CTRL + 0x18)
#define REG_PG4_PAD_CTRL_SPI_NF_SIP_D0      (REG_BASE_TOP_PG4_PAD_CTRL + 0x1c)
#define REG_PG4_PAD_CTRL_SPI_NF_SIP_SCLK    (REG_BASE_TOP_PG4_PAD_CTRL + 0x20)
#define REG_PG4_PAD_CTRL_SPI_NF_SIP_D3      (REG_BASE_TOP_PG4_PAD_CTRL + 0x24)
#define REG_PG4_PAD_CTRL_UART1_TXD          (REG_BASE_TOP_PG4_PAD_CTRL + 0x28)
#define REG_PG4_PAD_CTRL_UART1_RXD          (REG_BASE_TOP_PG4_PAD_CTRL + 0x2c)
#define REG_PG4_PAD_CTRL_UART3_TXD          (REG_BASE_TOP_PG4_PAD_CTRL + 0x30)
#define REG_PG4_PAD_CTRL_UART3_RXD          (REG_BASE_TOP_PG4_PAD_CTRL + 0x34)

/* pin group5 */
#define REG_PG5_PAD_CTRL_SPI_LCD_SDI        (REG_BASE_TOP_PG5_PAD_CTRL + 0x10)
#define REG_PG5_PAD_CTRL_SPI_LCD_SDO        (REG_BASE_TOP_PG5_PAD_CTRL + 0x14)
#define REG_PG5_PAD_CTRL_SPI_LCD_DCX        (REG_BASE_TOP_PG5_PAD_CTRL + 0x18)
#define REG_PG5_PAD_CTRL_SPI_LCD_CS         (REG_BASE_TOP_PG5_PAD_CTRL + 0x1c)
#define REG_PG5_PAD_CTRL_SPI_LCD_CLK        (REG_BASE_TOP_PG5_PAD_CTRL + 0x20)
#define REG_PG5_PAD_CTRL_LCD_FMARK          (REG_BASE_TOP_PG5_PAD_CTRL + 0x24)
#define REG_PG5_PAD_CTRL_LCD_RSTN           (REG_BASE_TOP_PG5_PAD_CTRL + 0x28)

/* pin group6 */
#define REG_PG6_PAD_CTRL_SDMMC_D0           (REG_BASE_TOP_PG6_PAD_CTRL + 0x10)
#define REG_PG6_PAD_CTRL_SDMMC_D1           (REG_BASE_TOP_PG6_PAD_CTRL + 0x14)
#define REG_PG6_PAD_CTRL_SDMMC_D2           (REG_BASE_TOP_PG6_PAD_CTRL + 0x18)
#define REG_PG6_PAD_CTRL_SDMMC_D3           (REG_BASE_TOP_PG6_PAD_CTRL + 0x1c)
#define REG_PG6_PAD_CTRL_SDMMC_CLK          (REG_BASE_TOP_PG6_PAD_CTRL + 0x20)
#define REG_PG6_PAD_CTRL_SDMMC_CMD          (REG_BASE_TOP_PG6_PAD_CTRL + 0x24)

/* pin group7 */
#define REG_PG7_PAD_CTRL_SDMMC_RST          (REG_BASE_TOP_PG7_PAD_CTRL + 0x10)
#define REG_PG7_PAD_CTRL_SDMMC_D4           (REG_BASE_TOP_PG7_PAD_CTRL + 0x14)
#define REG_PG7_PAD_CTRL_SDMMC_D5           (REG_BASE_TOP_PG7_PAD_CTRL + 0x18)
#define REG_PG7_PAD_CTRL_SDMMC_D6           (REG_BASE_TOP_PG7_PAD_CTRL + 0x1c)
#define REG_PG7_PAD_CTRL_SDMMC_D7           (REG_BASE_TOP_PG7_PAD_CTRL + 0x20)
#define REG_PG7_PAD_CTRL_SDMMC_REB          (REG_BASE_TOP_PG7_PAD_CTRL + 0x24)

/* pin group 8 */
#define REG_PG8_PAD_CTRL_SIM1_RST           (REG_BASE_TOP_PG8_PAD_CTRL + 0x10)
#define REG_PG8_PAD_CTRL_SIM1_DAT           (REG_BASE_TOP_PG8_PAD_CTRL + 0x14)
#define REG_PG8_PAD_CTRL_SIM1_CLK           (REG_BASE_TOP_PG8_PAD_CTRL + 0x18)

/* pin group 9 */
#define REG_PG9_PAD_CTRL_SIM0_RST           (REG_BASE_TOP_PG9_PAD_CTRL + 0x10)
#define REG_PG9_PAD_CTRL_SIM0_DAT           (REG_BASE_TOP_PG9_PAD_CTRL + 0x14)
#define REG_PG9_PAD_CTRL_SIM0_CLK           (REG_BASE_TOP_PG9_PAD_CTRL + 0x18)

/* pin group 10 */
#define REG_PG10_PAD_CTRL_UART0_TXD         (REG_BASE_TOP_PG10_PAD_CTRL + 0x10)
#define REG_PG10_PAD_CTRL_UART0_RXD         (REG_BASE_TOP_PG10_PAD_CTRL + 0x14)
#define REG_PG10_PAD_CTRL_UART0_CTS         (REG_BASE_TOP_PG10_PAD_CTRL + 0x18)
#define REG_PG10_PAD_CTRL_UART0_RTS         (REG_BASE_TOP_PG10_PAD_CTRL + 0x1c)
#define REG_PG10_PAD_CTRL_SSP1_CS1          (REG_BASE_TOP_PG10_PAD_CTRL + 0x20)
#define REG_PG10_PAD_CTRL_SSP1_CLK          (REG_BASE_TOP_PG10_PAD_CTRL + 0x24)
#define REG_PG10_PAD_CTRL_SSP1_CS0          (REG_BASE_TOP_PG10_PAD_CTRL + 0x28)
#define REG_PG10_PAD_CTRL_SSP1_MISO         (REG_BASE_TOP_PG10_PAD_CTRL + 0x2c)
#define REG_PG10_PAD_CTRL_SSP1_MOSI         (REG_BASE_TOP_PG10_PAD_CTRL + 0x30)

/* pin group 11 */
#define REG_PG11_PAD_CTRL_IIS0_MCLK         (REG_BASE_TOP_PG11_PAD_CTRL + 0x10)
#define REG_PG11_PAD_CTRL_IIS0_CLK          (REG_BASE_TOP_PG11_PAD_CTRL + 0x14)
#define REG_PG11_PAD_CTRL_IIS0_LRCLK        (REG_BASE_TOP_PG11_PAD_CTRL + 0x18)
#define REG_PG11_PAD_CTRL_IIS0_DI           (REG_BASE_TOP_PG11_PAD_CTRL + 0x1c)
#define REG_PG11_PAD_CTRL_IIS0_DO           (REG_BASE_TOP_PG11_PAD_CTRL + 0x20)
#define REG_PG11_PAD_CTRL_SSP0_CLK          (REG_BASE_TOP_PG11_PAD_CTRL + 0x24)
#define REG_PG11_PAD_CTRL_SSP0_CS0          (REG_BASE_TOP_PG11_PAD_CTRL + 0x28)
#define REG_PG11_PAD_CTRL_SSP0_MISO         (REG_BASE_TOP_PG11_PAD_CTRL + 0x2c)
#define REG_PG11_PAD_CTRL_SSP0_MOSI         (REG_BASE_TOP_PG11_PAD_CTRL + 0x30)

/* pin group 12 */
#define REG_PG12_PAD_CTRL_KEYIN4            (REG_BASE_TOP_PG12_PAD_CTRL + 0x10)
#define REG_PG12_PAD_CTRL_KEYIN5            (REG_BASE_TOP_PG12_PAD_CTRL + 0x14)
#define REG_PG12_PAD_CTRL_KEYOUT4           (REG_BASE_TOP_PG12_PAD_CTRL + 0x18)
#define REG_PG12_PAD_CTRL_KEYOUT5           (REG_BASE_TOP_PG12_PAD_CTRL + 0x1c)
#define REG_PG12_PAD_CTRL_KEYIN0            (REG_BASE_TOP_PG12_PAD_CTRL + 0x20)
#define REG_PG12_PAD_CTRL_KEYIN1            (REG_BASE_TOP_PG12_PAD_CTRL + 0x24)
#define REG_PG12_PAD_CTRL_KEYIN2            (REG_BASE_TOP_PG12_PAD_CTRL + 0x28)
#define REG_PG12_PAD_CTRL_KEYIN3            (REG_BASE_TOP_PG12_PAD_CTRL + 0x2c)
#define REG_PG12_PAD_CTRL_KEYOUT0           (REG_BASE_TOP_PG12_PAD_CTRL + 0x30)
#define REG_PG12_PAD_CTRL_KEYOUT1           (REG_BASE_TOP_PG12_PAD_CTRL + 0x34)
#define REG_PG12_PAD_CTRL_KEYOUT2           (REG_BASE_TOP_PG12_PAD_CTRL + 0x38)
#define REG_PG12_PAD_CTRL_KEYOUT3           (REG_BASE_TOP_PG12_PAD_CTRL + 0x3c)

/* pin group 13 */
#define REG_PG13_PAD_CTRL_RFCTRL0           (REG_BASE_TOP_PG13_PAD_CTRL + 0x10)
#define REG_PG13_PAD_CTRL_RFCTRL1           (REG_BASE_TOP_PG13_PAD_CTRL + 0x14)
#define REG_PG13_PAD_CTRL_RFCTRL2           (REG_BASE_TOP_PG13_PAD_CTRL + 0x18)
#define REG_PG13_PAD_CTRL_RFCTRL3           (REG_BASE_TOP_PG13_PAD_CTRL + 0x1c)
#define REG_PG13_PAD_CTRL_RFCTRL4           (REG_BASE_TOP_PG13_PAD_CTRL + 0x20)
#define REG_PG13_PAD_CTRL_RFCTRL5           (REG_BASE_TOP_PG13_PAD_CTRL + 0x24)
#define REG_PG13_PAD_CTRL_RFCTRL6           (REG_BASE_TOP_PG13_PAD_CTRL + 0x28)
#define REG_PG13_PAD_CTRL_RFCTRL7           (REG_BASE_TOP_PG13_PAD_CTRL + 0x2c)

/* bit fields definition */
#define BIT_PAD_SLP_TOP                 BIT_19
#define BIT_PAD_SLP_AUD                 BIT_18
#define BIT_PAD_SLP_CP                  BIT_17
#define BIT_PAD_SLP_MCU                 BIT_16
#define BIT_PAD_SLP_ALL                 BIT_PAD_SLP_MCU|BIT_PAD_SLP_AUD|BIT_PAD_SLP_CP|BIT_PAD_SLP_TOP
#define BIT_PAD_SLP_NONE                0
#define BIT_PAD_SLP_WPU                 (0x3 << 11)
#define BIT_PAD_SLP_SPU                 (0x2 << 11)
#define BIT_PAD_SLP_SSPU                (0x1 << 11)
#define BIT_PAD_SLP_WPD                 BIT_10
#define BIT_PAD_SLP_PX                  0
#define BIT_PAD_SLP_IE                  BIT_9
#define BIT_PAD_SLP_OE                  BIT_8
#define BIT_PAD_SLP_CH                  BIT_8|BIT_13|BIT_14
#define BIT_PAD_SLP_CL                  BIT_8|BIT_14
#define BIT_PAD_SLP_Z                   0
#define BIT_PAD_SE                      BIT_6
#define BIT_PAD_WPU                     (0x3 << 4)
#define BIT_PAD_SPU                     (0x2 << 4)
#define BIT_PAD_SSPU                    (0x1 << 4)
#define BIT_PAD_WPD                     BIT_3
#define BIT_PAD_PX                      0
#define BIT_PAD_DRV(x)                  (((x) & 7))
#define BIT_FUNC_SEL(x)                 (((x) & 7))
#define BIT_PAD_PWR_AUTO                BIT_0
#define BIT_PAD_PWR_MANUL_1V8           BIT_1
#define BIT_PAD_PWR_MANUL_3V3           0

#define BIT_PMIC_PAD_DRV(x)             (((x) & 1))  //PMIC PAD driver strength select
#define BIT_PMIC_PAD_WPU                BIT_1|BIT_2 //PMIC PAD weak pull up
#define BIT_PMIC_PAD_WPD                BIT_2       //PMIC PAD weak pull down
#define BIT_PMIC_PAD_PX                 0            //PMIC PAD no pull up and no pull down
#define BIT_PMIC_PAD_SE                 BIT_3        //PMIC PAD schmitt trigger enable
#endif /* __PINLIST_H__ */
