/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __GPO_CHIP_CFG_H__
#define __GPO_CHIP_CFG_H__

#include <aic_auto_reg.h>
#include <aic_reg_base.h>
#include <pinlist.h>

#define GPO_PIN_EB                      BIT(22)
#define GPO_PIN_VAL                     BIT(21)

const static u32 g_gpo_map[] = {
    /* PAD CTRL */
    /* Group5 */
    REG_PG5_PAD_CTRL_GPO0,            //GPO0 (AON)
    REG_PG5_PAD_CTRL_GPO1,            //GPO1 (AON)
    REG_PG5_PAD_CTRL_EIU_INT4,        //GPO2 (AON)
    REG_PG5_PAD_CTRL_EIU_INT5,        //GPO3 (AON)
    REG_PG5_PAD_CTRL_EIU_INT6,        //GPO4 (AON)
    REG_PG5_PAD_CTRL_EIU_INT7,        //GPO5 (AON)
    REG_PG5_PAD_CTRL_SWD_CLK,         //GPO6 (AON)
    REG_PG5_PAD_CTRL_SWD_IO,          //GPO7 (AON)
    /* Group6 */
    //REG_PG6_PAD_CTRL_I2C0_SDA,
    //REG_PG6_PAD_CTRL_I2C0_SCL,
    //REG_PG6_PAD_CTRL_RST_REQ,
    //REG_PG6_PAD_CTRL_CLK26M_EN0,
    //REG_PG6_PAD_CTRL_BB_32K_IN,
    //REG_PG6_PAD_CTRL_PMIC_INT,
    //REG_PG6_PAD_CTRL_CHIP_SLEEP,
    REG_PG6_PAD_CTRL_UART0_TXD,       //GPO8 (ND)
    REG_PG6_PAD_CTRL_UART0_RXD,       //GPO9 (ND)
    REG_PG6_PAD_CTRL_UART0_CTS,       //GPO10 (ND)
    REG_PG6_PAD_CTRL_UART0_RTS,       //GPO11 (ND)
    REG_PG6_PAD_CTRL_EIU_INT0,        //GPO12 (ND)
    REG_PG6_PAD_CTRL_EIU_INT3,        //GPO13 (ND)
    REG_PG6_PAD_CTRL_EIU_INT2,        //GPO14 (ND)
    /* Group1 */
    REG_PG1_PAD_CTRL_RFCTRL0,         //GPO15 (PD)
    REG_PG1_PAD_CTRL_RFCTRL1,         //GPO16 (PD)
    REG_PG1_PAD_CTRL_RFCTRL2,         //GPO17 (PD)
    REG_PG1_PAD_CTRL_RFCTRL3,         //GPO18 (PD)
    REG_PG1_PAD_CTRL_RFCTRL4,         //GPO19 (PD)
    REG_PG1_PAD_CTRL_RFCTRL5,         //GPO20 (PD)
    REG_PG1_PAD_CTRL_RFCTRL6,         //GPO21 (PD)
    REG_PG1_PAD_CTRL_RFCTRL7,         //GPO22 (PD)
    /* Group2 */
    REG_PG2_PAD_CTRL_IIS_MCLK,        //GPO23 (PD)
    REG_PG2_PAD_CTRL_IIS_CLK,         //GPO24 (PD)
    REG_PG2_PAD_CTRL_IIS_LRCLK,       //GPO25 (PD)
    REG_PG2_PAD_CTRL_IIS_DI,          //GPO26 (PD)
    REG_PG2_PAD_CTRL_IIS_DO,          //GPO27 (PD)
    REG_PG2_PAD_CTRL_UART1_RXD,       //GPO28 (PD)
    REG_PG2_PAD_CTRL_UART1_TXD,       //GPO29 (PD)
    /* Group3 */
    REG_PG3_PAD_CTRL_SSP_CS1,         //GPO30 (PD)
    REG_PG3_PAD_CTRL_SSP_CLK,         //GPO31 (PD)
    REG_PG3_PAD_CTRL_SSP_CS0,         //GPO32 (PD)
    REG_PG3_PAD_CTRL_SSP_MISO,        //GPO33 (PD)
    REG_PG3_PAD_CTRL_SSP_MOSI,        //GPO34 (PD)
    REG_PG3_PAD_CTRL_AUX_CLK0,        //GPO35 (PD)
    /* Group7 */
    REG_PG7_PAD_CTRL_I2C1_SDA,        //GPO36 (PD)
    REG_PG7_PAD_CTRL_I2C1_SCL,        //GPO37 (PD)
    REG_PG7_PAD_CTRL_CAM_MCLK,        //GPO38 (PD)
    REG_PG7_PAD_CTRL_CAM_SPI_D0,      //GPO39 (PD)
    REG_PG7_PAD_CTRL_GPIO0,           //GPO40 (PD)
    REG_PG7_PAD_CTRL_CAM_SPI_CLK,     //GPO41 (PD)
    REG_PG7_PAD_CTRL_GPIO1,           //GPO42 (PD)
    REG_PG7_PAD_CTRL_CAM_SPI_D1,      //GPO43 (PD)
    /* Group9 */
    REG_PG9_PAD_CTRL_SIM_RST,         //GPO44 (PD)
    REG_PG9_PAD_CTRL_SIM_CLK,         //GPO45 (PD)
    REG_PG9_PAD_CTRL_SIM_DAT,         //GPO46 (PD)
    /* Group10 */
    REG_PG10_PAD_CTRL_CAN_TXD,        //GPO47 (PD)
    REG_PG10_PAD_CTRL_CAN_RXD,        //GPO48 (PD)
    REG_PG10_PAD_CTRL_CAN_STBY,       //GPO49 (PD)
    /* Group11 */
    REG_PG11_PAD_CTRL_SPI_LCD_SDI,    //GPO50 (PD)
    REG_PG11_PAD_CTRL_SPI_LCD_SDO,    //GPO51 (PD)
    REG_PG11_PAD_CTRL_SPI_LCD_DCX,    //GPO52 (PD)
    REG_PG11_PAD_CTRL_SPI_LCD_CS,     //GPO53 (PD)
    REG_PG11_PAD_CTRL_SPI_LCD_CLK,    //GPO54 (PD)
    REG_PG11_PAD_CTRL_LCD_FMARK,      //GPO55 (PD)
    REG_PG11_PAD_CTRL_LCD_RSTN,       //GPO56 (PD)
};

#endif /* __GPO_CHIP_CFG_H__ */

