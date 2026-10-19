/*****************************************************************************/
/*                                                                           */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
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
#include <aic_reg_base.h>

/* Cross Bar */
#define REG_IOMUX_IO_CROSSBAR_SEL_UART      (REG_BASE_IO_PD_IOMUX + 0x10)
#define REG_IOMUX_IO_CROSSBAR_SEL_IIC       (REG_BASE_IO_PD_IOMUX + 0x14)
#define REG_IOMUX_IO_CROSSBAR_SEL_RFFE      (REG_BASE_IO_PD_IOMUX + 0x18)

/* PAD Power CTRL */
#define REG_PG3_PAD_CTRL_PWR_PAD_CTRL       (REG_BASE_PD_PG3_PAD_CTRL + 0x0)
#define REG_PG5_PAD_CTRL_PWR_PAD_CTRL       (REG_BASE_PD_PG5_PAD_CTRL + 0x0)
#define REG_PG9_PAD_CTRL_PWR_PAD_CTRL       (REG_BASE_PD_PG9_PAD_CTRL + 0x0)
#define REG_PG10_PAD_CTRL_PWR_PAD_CTRL      (REG_BASE_PD_PG10_PAD_CTRL + 0x0)
#define REG_PG11_PAD_CTRL_PWR_PAD_CTRL      (REG_BASE_PD_PG11_PAD_CTRL + 0x0)

/* io mux */
#define REG_IOMUX_RFCTRL0                   (REG_BASE_IO_PD_IOMUX + 0x1c)
#define REG_IOMUX_RFCTRL1                   (REG_BASE_IO_PD_IOMUX + 0x20)
#define REG_IOMUX_RFCTRL2                   (REG_BASE_IO_PD_IOMUX + 0x24)
#define REG_IOMUX_RFCTRL3                   (REG_BASE_IO_PD_IOMUX + 0x28)
#define REG_IOMUX_RFCTRL4                   (REG_BASE_IO_PD_IOMUX + 0x2c)
#define REG_IOMUX_RFCTRL5                   (REG_BASE_IO_PD_IOMUX + 0x30)
#define REG_IOMUX_RFCTRL6                   (REG_BASE_IO_PD_IOMUX + 0x34)
#define REG_IOMUX_RFCTRL7                   (REG_BASE_IO_PD_IOMUX + 0x38)
#define REG_IOMUX_IIS_MCLK                  (REG_BASE_IO_PD_IOMUX + 0x3c)
#define REG_IOMUX_IIS_CLK                   (REG_BASE_IO_PD_IOMUX + 0x40)
#define REG_IOMUX_IIS_LRCLK                 (REG_BASE_IO_PD_IOMUX + 0x44)
#define REG_IOMUX_IIS_DI                    (REG_BASE_IO_PD_IOMUX + 0x48)
#define REG_IOMUX_IIS_DO                    (REG_BASE_IO_PD_IOMUX + 0x4c)
#define REG_IOMUX_UART1_RXD                 (REG_BASE_IO_PD_IOMUX + 0x50)
#define REG_IOMUX_UART1_TXD                 (REG_BASE_IO_PD_IOMUX + 0x54)
#define REG_IOMUX_SSP_CS1                   (REG_BASE_IO_PD_IOMUX + 0x58)
#define REG_IOMUX_SSP_CLK                   (REG_BASE_IO_PD_IOMUX + 0x5c)
#define REG_IOMUX_SSP_CS0                   (REG_BASE_IO_PD_IOMUX + 0x60)
#define REG_IOMUX_SSP_MISO                  (REG_BASE_IO_PD_IOMUX + 0x64)
#define REG_IOMUX_SSP_MOSI                  (REG_BASE_IO_PD_IOMUX + 0x68)
#define REG_IOMUX_AUX_CLK0                  (REG_BASE_IO_PD_IOMUX + 0x6c)
#define REG_IOMUX_GPO0                      (REG_BASE_PD_PG5_PAD_CTRL + 0x30)
#define REG_IOMUX_GPO1                      (REG_BASE_PD_PG5_PAD_CTRL + 0x34)
#define REG_IOMUX_EIU_INT4                  (REG_BASE_PD_PG5_PAD_CTRL + 0x38)
#define REG_IOMUX_EIU_INT5                  (REG_BASE_PD_PG5_PAD_CTRL + 0x3c)
#define REG_IOMUX_EIU_INT6                  (REG_BASE_PD_PG5_PAD_CTRL + 0x40)
#define REG_IOMUX_EIU_INT7                  (REG_BASE_PD_PG5_PAD_CTRL + 0x44)
#define REG_IOMUX_SWD_CLK                   (REG_BASE_PD_PG5_PAD_CTRL + 0x48)
#define REG_IOMUX_SWD_IO                    (REG_BASE_PD_PG5_PAD_CTRL + 0x4c)
#define REG_IOMUX_I2C0_SDA                  (REG_BASE_SYS_ND_GLB + 0x7050)
#define REG_IOMUX_I2C0_SCL                  (REG_BASE_SYS_ND_GLB + 0x7054)
#define REG_IOMUX_RST_REQ                   (REG_BASE_SYS_ND_GLB + 0x7058)
#define REG_IOMUX_CLK26M_EN0                (REG_BASE_SYS_ND_GLB + 0x705c)
#define REG_IOMUX_PMIC_INT                  (REG_BASE_SYS_ND_GLB + 0x7060)
#define REG_IOMUX_CHIP_SLEEP                (REG_BASE_SYS_ND_GLB + 0x7064)
#define REG_IOMUX_UART0_TXD                 (REG_BASE_SYS_ND_GLB + 0x7068)
#define REG_IOMUX_UART0_RXD                 (REG_BASE_SYS_ND_GLB + 0x706c)
#define REG_IOMUX_UART0_CTS                 (REG_BASE_SYS_ND_GLB + 0x7070)
#define REG_IOMUX_UART0_RTS                 (REG_BASE_SYS_ND_GLB + 0x7074)
#define REG_IOMUX_EIU_INT0                  (REG_BASE_SYS_ND_GLB + 0x7078)
#define REG_IOMUX_EIU_INT3                  (REG_BASE_SYS_ND_GLB + 0x707c)
#define REG_IOMUX_EIU_INT2                  (REG_BASE_SYS_ND_GLB + 0x7080)
#define REG_IOMUX_I2C1_SDA                  (REG_BASE_IO_PD_IOMUX + 0x70)
#define REG_IOMUX_I2C1_SCL                  (REG_BASE_IO_PD_IOMUX + 0x74)
#define REG_IOMUX_CAM_MCLK                  (REG_BASE_IO_PD_IOMUX + 0x78)
#define REG_IOMUX_CAM_SPI_D0                (REG_BASE_IO_PD_IOMUX + 0x7c)
#define REG_IOMUX_GPIO0                     (REG_BASE_IO_PD_IOMUX + 0x80)
#define REG_IOMUX_CAM_SPI_CLK               (REG_BASE_IO_PD_IOMUX + 0x84)
#define REG_IOMUX_GPIO1                     (REG_BASE_IO_PD_IOMUX + 0x88)
#define REG_IOMUX_CAM_SPI_D1                (REG_BASE_IO_PD_IOMUX + 0x8c)
#define REG_IOMUX_SPI_NF_SIP_D0             (REG_BASE_IO_PD_IOMUX + 0x90)
#define REG_IOMUX_SPI_NF_SIP_SCLK           (REG_BASE_IO_PD_IOMUX + 0x94)
#define REG_IOMUX_SPI_NF_SIP_D3             (REG_BASE_IO_PD_IOMUX + 0x98)
#define REG_IOMUX_SPI_NF_SIP_CS0            (REG_BASE_IO_PD_IOMUX + 0x9c)
#define REG_IOMUX_SPI_NF_SIP_D1             (REG_BASE_IO_PD_IOMUX + 0xa0)
#define REG_IOMUX_SPI_NF_SIP_D2             (REG_BASE_IO_PD_IOMUX + 0xa4)
#define REG_IOMUX_SIM_RST                   (REG_BASE_IO_PD_IOMUX + 0xa8)
#define REG_IOMUX_SIM_CLK                   (REG_BASE_IO_PD_IOMUX + 0xac)
#define REG_IOMUX_SIM_DAT                   (REG_BASE_IO_PD_IOMUX + 0xb0)
#define REG_IOMUX_CAN_TXD                   (REG_BASE_IO_PD_IOMUX + 0xb4)
#define REG_IOMUX_CAN_RXD                   (REG_BASE_IO_PD_IOMUX + 0xb8)
#define REG_IOMUX_CAN_STBY                  (REG_BASE_IO_PD_IOMUX + 0xbc)
#define REG_IOMUX_SPI_LCD_SDI               (REG_BASE_IO_PD_IOMUX + 0xc0)
#define REG_IOMUX_SPI_LCD_SDO               (REG_BASE_IO_PD_IOMUX + 0xc4)
#define REG_IOMUX_SPI_LCD_DCX               (REG_BASE_IO_PD_IOMUX + 0xc8)
#define REG_IOMUX_SPI_LCD_CS                (REG_BASE_IO_PD_IOMUX + 0xcc)
#define REG_IOMUX_SPI_LCD_CLK               (REG_BASE_IO_PD_IOMUX + 0xd0)
#define REG_IOMUX_LCD_FMARK                 (REG_BASE_IO_PD_IOMUX + 0xd4)
#define REG_IOMUX_LCD_RSTN                  (REG_BASE_IO_PD_IOMUX + 0xd8)

/* PAD CTRL */
/* pin group1 */
#define REG_PG1_PAD_CTRL_RFCTRL0            (REG_BASE_PD_PG1_PAD_CTRL + 0x10)
#define REG_PG1_PAD_CTRL_RFCTRL1            (REG_BASE_PD_PG1_PAD_CTRL + 0x14)
#define REG_PG1_PAD_CTRL_RFCTRL2            (REG_BASE_PD_PG1_PAD_CTRL + 0x18)
#define REG_PG1_PAD_CTRL_RFCTRL3            (REG_BASE_PD_PG1_PAD_CTRL + 0x1c)
#define REG_PG1_PAD_CTRL_RFCTRL4            (REG_BASE_PD_PG1_PAD_CTRL + 0x20)
#define REG_PG1_PAD_CTRL_RFCTRL5            (REG_BASE_PD_PG1_PAD_CTRL + 0x24)
#define REG_PG1_PAD_CTRL_RFCTRL6            (REG_BASE_PD_PG1_PAD_CTRL + 0x28)
#define REG_PG1_PAD_CTRL_RFCTRL7            (REG_BASE_PD_PG1_PAD_CTRL + 0x2c)

/* pin group2 */
#define REG_PG2_PAD_CTRL_IIS_MCLK           (REG_BASE_PD_PG2_PAD_CTRL + 0x10)
#define REG_PG2_PAD_CTRL_IIS_CLK            (REG_BASE_PD_PG2_PAD_CTRL + 0x14)
#define REG_PG2_PAD_CTRL_IIS_LRCLK          (REG_BASE_PD_PG2_PAD_CTRL + 0x18)
#define REG_PG2_PAD_CTRL_IIS_DI             (REG_BASE_PD_PG2_PAD_CTRL + 0x1c)
#define REG_PG2_PAD_CTRL_IIS_DO             (REG_BASE_PD_PG2_PAD_CTRL + 0x20)
#define REG_PG2_PAD_CTRL_UART1_RXD          (REG_BASE_PD_PG2_PAD_CTRL + 0x24)
#define REG_PG2_PAD_CTRL_UART1_TXD          (REG_BASE_PD_PG2_PAD_CTRL + 0x28)

/* pin group3 */
#define REG_PG3_PAD_CTRL_SSP_CS1            (REG_BASE_PD_PG3_PAD_CTRL + 0x10)
#define REG_PG3_PAD_CTRL_SSP_CLK            (REG_BASE_PD_PG3_PAD_CTRL + 0x14)
#define REG_PG3_PAD_CTRL_SSP_CS0            (REG_BASE_PD_PG3_PAD_CTRL + 0x18)
#define REG_PG3_PAD_CTRL_SSP_MISO           (REG_BASE_PD_PG3_PAD_CTRL + 0x1c)
#define REG_PG3_PAD_CTRL_SSP_MOSI           (REG_BASE_PD_PG3_PAD_CTRL + 0x20)
#define REG_PG3_PAD_CTRL_AUX_CLK0           (REG_BASE_PD_PG3_PAD_CTRL + 0x24)

/* pin group4 */
#define REG_PG4_PAD_CTRL_PSRAM_RWDS         (REG_BASE_PD_PG4_PAD_CTRL + 0x10)
#define REG_PG4_PAD_CTRL_PSRAM_DQ7          (REG_BASE_PD_PG4_PAD_CTRL + 0x14)
#define REG_PG4_PAD_CTRL_PSRAM_DQ6          (REG_BASE_PD_PG4_PAD_CTRL + 0x18)
#define REG_PG4_PAD_CTRL_PSRAM_DQ5          (REG_BASE_PD_PG4_PAD_CTRL + 0x1c)
#define REG_PG4_PAD_CTRL_PSRAM_DQ4          (REG_BASE_PD_PG4_PAD_CTRL + 0x20)
#define REG_PG4_PAD_CTRL_PSRAM_CLK_P        (REG_BASE_PD_PG4_PAD_CTRL + 0x24)
#define REG_PG4_PAD_CTRL_PSRAM_CLK_N        (REG_BASE_PD_PG4_PAD_CTRL + 0x28)
#define REG_PG4_PAD_CTRL_PSRAM_CE           (REG_BASE_PD_PG4_PAD_CTRL + 0x2c)
#define REG_PG4_PAD_CTRL_PSRAM_DQ3          (REG_BASE_PD_PG4_PAD_CTRL + 0x30)
#define REG_PG4_PAD_CTRL_PSRAM_DQ2          (REG_BASE_PD_PG4_PAD_CTRL + 0x34)
#define REG_PG4_PAD_CTRL_PSRAM_DQ1          (REG_BASE_PD_PG4_PAD_CTRL + 0x38)
#define REG_PG4_PAD_CTRL_PSRAM_DQ0          (REG_BASE_PD_PG4_PAD_CTRL + 0x3c)
#define REG_PG4_PAD_CTRL_PSRAM_DM           (REG_BASE_PD_PG4_PAD_CTRL + 0x40)
#define REG_PG4_PAD_CTRL_PSRAM_RESETB       (REG_BASE_PD_PG4_PAD_CTRL + 0x44)

/* pin group5 */
#define REG_PG5_PAD_CTRL_GPO0               (REG_BASE_PD_PG5_PAD_CTRL + 0x10)
#define REG_PG5_PAD_CTRL_GPO1               (REG_BASE_PD_PG5_PAD_CTRL + 0x14)
#define REG_PG5_PAD_CTRL_EIU_INT4           (REG_BASE_PD_PG5_PAD_CTRL + 0x18)
#define REG_PG5_PAD_CTRL_EIU_INT5           (REG_BASE_PD_PG5_PAD_CTRL + 0x1c)
#define REG_PG5_PAD_CTRL_EIU_INT6           (REG_BASE_PD_PG5_PAD_CTRL + 0x20)
#define REG_PG5_PAD_CTRL_EIU_INT7           (REG_BASE_PD_PG5_PAD_CTRL + 0x24)
#define REG_PG5_PAD_CTRL_SWD_CLK            (REG_BASE_PD_PG5_PAD_CTRL + 0x28)
#define REG_PG5_PAD_CTRL_SWD_IO             (REG_BASE_PD_PG5_PAD_CTRL + 0x2c)

/* pin group6 */
#define REG_PG6_PAD_CTRL_I2C0_SDA           (REG_BASE_SYS_ND_GLB + 0x7010)
#define REG_PG6_PAD_CTRL_I2C0_SCL           (REG_BASE_SYS_ND_GLB + 0x7014)
#define REG_PG6_PAD_CTRL_RST_REQ            (REG_BASE_SYS_ND_GLB + 0x7018)
#define REG_PG6_PAD_CTRL_CLK26M_EN0         (REG_BASE_SYS_ND_GLB + 0x701c)
#define REG_PG6_PAD_CTRL_BB_32K_IN          (REG_BASE_SYS_ND_GLB + 0x7020)
#define REG_PG6_PAD_CTRL_PMIC_INT           (REG_BASE_SYS_ND_GLB + 0x7024)
#define REG_PG6_PAD_CTRL_CHIP_SLEEP         (REG_BASE_SYS_ND_GLB + 0x7028)
#define REG_PG6_PAD_CTRL_UART0_TXD          (REG_BASE_SYS_ND_GLB + 0x702c)
#define REG_PG6_PAD_CTRL_UART0_RXD          (REG_BASE_SYS_ND_GLB + 0x7030)
#define REG_PG6_PAD_CTRL_UART0_CTS          (REG_BASE_SYS_ND_GLB + 0x7034)
#define REG_PG6_PAD_CTRL_UART0_RTS          (REG_BASE_SYS_ND_GLB + 0x7038)
#define REG_PG6_PAD_CTRL_EIU_INT0           (REG_BASE_SYS_ND_GLB + 0x703c)
#define REG_PG6_PAD_CTRL_EIU_INT3           (REG_BASE_SYS_ND_GLB + 0x7040)
#define REG_PG6_PAD_CTRL_EIU_INT2           (REG_BASE_SYS_ND_GLB + 0x7044)

/* pin group7 */
#define REG_PG7_PAD_CTRL_I2C1_SDA           (REG_BASE_PD_PG7_PAD_CTRL + 0x10)
#define REG_PG7_PAD_CTRL_I2C1_SCL           (REG_BASE_PD_PG7_PAD_CTRL + 0x14)
#define REG_PG7_PAD_CTRL_CAM_MCLK           (REG_BASE_PD_PG7_PAD_CTRL + 0x18)
#define REG_PG7_PAD_CTRL_CAM_SPI_D0         (REG_BASE_PD_PG7_PAD_CTRL + 0x1c)
#define REG_PG7_PAD_CTRL_GPIO0              (REG_BASE_PD_PG7_PAD_CTRL + 0x20)
#define REG_PG7_PAD_CTRL_CAM_SPI_CLK        (REG_BASE_PD_PG7_PAD_CTRL + 0x24)
#define REG_PG7_PAD_CTRL_GPIO1              (REG_BASE_PD_PG7_PAD_CTRL + 0x28)
#define REG_PG7_PAD_CTRL_CAM_SPI_D1         (REG_BASE_PD_PG7_PAD_CTRL + 0x2c)

/* pin group 8 */
#define REG_PG8_PAD_CTRL_SPI_NF_SIP_D0      (REG_BASE_PD_PG8_PAD_CTRL + 0x10)
#define REG_PG8_PAD_CTRL_SPI_NF_SIP_SCLK    (REG_BASE_PD_PG8_PAD_CTRL + 0x14)
#define REG_PG8_PAD_CTRL_SPI_NF_SIP_D3      (REG_BASE_PD_PG8_PAD_CTRL + 0x18)
#define REG_PG8_PAD_CTRL_SPI_NF_SIP_CS0     (REG_BASE_PD_PG8_PAD_CTRL + 0x1c)
#define REG_PG8_PAD_CTRL_SPI_NF_SIP_D1      (REG_BASE_PD_PG8_PAD_CTRL + 0x20)
#define REG_PG8_PAD_CTRL_SPI_NF_SIP_D2      (REG_BASE_PD_PG8_PAD_CTRL + 0x24)

/* pin group 9 */
#define REG_PG9_PAD_CTRL_SIM_RST            (REG_BASE_PD_PG9_PAD_CTRL + 0x10)
#define REG_PG9_PAD_CTRL_SIM_CLK            (REG_BASE_PD_PG9_PAD_CTRL + 0x14)
#define REG_PG9_PAD_CTRL_SIM_DAT            (REG_BASE_PD_PG9_PAD_CTRL + 0x18)

/* pin group 10 */
#define REG_PG10_PAD_CTRL_CAN_TXD           (REG_BASE_PD_PG10_PAD_CTRL + 0x10)
#define REG_PG10_PAD_CTRL_CAN_RXD           (REG_BASE_PD_PG10_PAD_CTRL + 0x14)
#define REG_PG10_PAD_CTRL_CAN_STBY          (REG_BASE_PD_PG10_PAD_CTRL + 0x18)

/* pin group 11 */
#define REG_PG11_PAD_CTRL_SPI_LCD_SDI       (REG_BASE_PD_PG11_PAD_CTRL + 0x10)
#define REG_PG11_PAD_CTRL_SPI_LCD_SDO       (REG_BASE_PD_PG11_PAD_CTRL + 0x14)
#define REG_PG11_PAD_CTRL_SPI_LCD_DCX       (REG_BASE_PD_PG11_PAD_CTRL + 0x18)
#define REG_PG11_PAD_CTRL_SPI_LCD_CS        (REG_BASE_PD_PG11_PAD_CTRL + 0x1c)
#define REG_PG11_PAD_CTRL_SPI_LCD_CLK       (REG_BASE_PD_PG11_PAD_CTRL + 0x20)
#define REG_PG11_PAD_CTRL_LCD_FMARK         (REG_BASE_PD_PG11_PAD_CTRL + 0x24)
#define REG_PG11_PAD_CTRL_LCD_RSTN          (REG_BASE_PD_PG11_PAD_CTRL + 0x28)

/* bit fields definition */

#define BIT_PAD_GPOH                    BIT_21|BIT_22
#define BIT_PAD_GPOL                    BIT_22
#define BIT_PAD_SLP_TOP                 BIT_20
#define BIT_PAD_SLP_MODEM               BIT_19
#define BIT_PAD_SLP_MEM                 BIT_18
#define BIT_PAD_SLP_MCU                 BIT_17
#define BIT_PAD_SLP_CP                  BIT_16
#define BIT_PAD_SLP_ALL                 BIT_PAD_SLP_MCU|BIT_PAD_SLP_MODEM|BIT_PAD_SLP_CP|BIT_PAD_SLP_TOP|BIT_PAD_SLP_MEM
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

#endif /* __PINLIST_H__ */
