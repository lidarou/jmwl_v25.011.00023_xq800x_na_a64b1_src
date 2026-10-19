/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * PMIC pinlist.
 */

#ifndef __PMIC_PINLIST_H__
#define __PMIC_PINLIST_H__

#include <aic_bitmap.h>

/* GP201 macro definitions */
#define GP201_PADCTRL_BASE0                    (0x00000E00)
#define GP201_PADCTRL_BASE1                    (0x00009000)
#define GP201_IO_MUX_PON_N                     (GP201_PADCTRL_BASE0 + 0x20)
#define GP201_PON_N                            (GP201_PADCTRL_BASE0 + 0x0)
#define GP201_IO_MUX_RSTIN_N                   (GP201_PADCTRL_BASE0 + 0x24)
#define GP201_RSTIN_N                          (GP201_PADCTRL_BASE0 + 0x4)
#define GP201_IO_MUX_XO_SET                    (GP201_PADCTRL_BASE0 + 0x28)
#define GP201_XO_SET                           (GP201_PADCTRL_BASE0 + 0x8)
#define GP201_IO_MUX_RTC_SET                   (GP201_PADCTRL_BASE0 + 0x2c)
#define GP201_RTC_SET                          (GP201_PADCTRL_BASE0 + 0xc)
#define GP201_IO_MUX_GPIO0                     (GP201_PADCTRL_BASE0 + 0x30)
#define GP201_GPIO0                            (GP201_PADCTRL_BASE0 + 0x10)
#define GP201_IO_MUX_GPIO1                     (GP201_PADCTRL_BASE0 + 0x34)
#define GP201_GPIO1                            (GP201_PADCTRL_BASE0 + 0x14)
#define GP201_IO_MUX_GPIO2                     (GP201_PADCTRL_BASE0 + 0x38)
#define GP201_GPIO2                            (GP201_PADCTRL_BASE0 + 0x18)
#define GP201_IO_MUX_GPIO5                     (GP201_PADCTRL_BASE1 + 0x40)
#define GP201_GPIO5                            (GP201_PADCTRL_BASE1 + 0x0)
#define GP201_IO_MUX_SLEEP_EN                  (GP201_PADCTRL_BASE1 + 0x44)
#define GP201_SLEEP_EN                         (GP201_PADCTRL_BASE1 + 0x4)
#define GP201_IO_MUX_GPIO4                     (GP201_PADCTRL_BASE1 + 0x48)
#define GP201_GPIO4                            (GP201_PADCTRL_BASE1 + 0x8)
#define GP201_IO_MUX_PMIC_INT                  (GP201_PADCTRL_BASE1 + 0x4c)
#define GP201_PMIC_INT                         (GP201_PADCTRL_BASE1 + 0xc)
#define GP201_IO_MUX_RST_REQ                   (GP201_PADCTRL_BASE1 + 0x50)
#define GP201_RST_REQ                          (GP201_PADCTRL_BASE1 + 0x10)
#define GP201_IO_MUX_PMIC_RSTN                 (GP201_PADCTRL_BASE1 + 0x54)
#define GP201_PMIC_RSTN                        (GP201_PADCTRL_BASE1 + 0x14)
#define GP201_IO_MUX_BB_32K                    (GP201_PADCTRL_BASE1 + 0x58)
#define GP201_BB_32K                           (GP201_PADCTRL_BASE1 + 0x18)
#define GP201_IO_MUX_IIC_CLK                   (GP201_PADCTRL_BASE1 + 0x5c)
#define GP201_IIC_CLK                          (GP201_PADCTRL_BASE1 + 0x1c)
#define GP201_IO_MUX_IIC_DAT                   (GP201_PADCTRL_BASE1 + 0x60)
#define GP201_IIC_DAT                          (GP201_PADCTRL_BASE1 + 0x20)
#define GP201_IO_MUX_GPIO3                     (GP201_PADCTRL_BASE1 + 0x64)
#define GP201_GPIO3                            (GP201_PADCTRL_BASE1 + 0x24)
#define GP201_IO_MUX_GPIO6                     (GP201_PADCTRL_BASE1 + 0x68)
#define GP201_GPIO6                            (GP201_PADCTRL_BASE1 + 0x28)
#define GP201_IO_MUX_GPIO7                     (GP201_PADCTRL_BASE1 + 0x6c)
#define GP201_GPIO7                            (GP201_PADCTRL_BASE1 + 0x2c)
#define GP201_IO_MUX_GPIO8                     (GP201_PADCTRL_BASE1 + 0x70)
#define GP201_GPIO8                            (GP201_PADCTRL_BASE1 + 0x30)
#define GP201_IO_MUX_GPIO9                     (GP201_PADCTRL_BASE1 + 0x74)
#define GP201_GPIO9                            (GP201_PADCTRL_BASE1 + 0x34)
#define GP201_IO_MUX_GPIO10                    (GP201_PADCTRL_BASE1 + 0x78)
#define GP201_GPIO10                           (GP201_PADCTRL_BASE1 + 0x38)
#define GP201_IO_MUX_GPIO11                    (GP201_PADCTRL_BASE1 + 0x7c)
#define GP201_GPIO11                           (GP201_PADCTRL_BASE1 + 0x3c)

#define BIT_GP201_PAD_DRV(x)                   (((x) & 1)) //PMIC PAD driver strength select
#define BIT_GP201_PAD_WPU                      BIT_1|BIT_2 //PMIC PAD weak pull up
#define BIT_GP201_PAD_WPD                      BIT_2       //PMIC PAD weak pull down
#define BIT_GP201_PAD_PX                       0           //PMIC PAD no pull up and no pull down
#define BIT_GP201_PAD_SE                       BIT_3       //PMIC PAD schmitt trigger enable

/* GP202 macro definitions */
#define GP202_PADCTRL_BASE0                    (0x00000600)
#define GP202_PADCTRL_BASE1                    (0x00009000)
#define GP202_IO_MUX_RSTIN_N                   (GP202_PADCTRL_BASE0 + 0x40)
#define GP202_RSTIN_N                          (GP202_PADCTRL_BASE0 + 0x10)
#define GP202_IO_MUX_GPIO0                     (GP202_PADCTRL_BASE0 + 0x44)
#define GP202_GPIO0                            (GP202_PADCTRL_BASE0 + 0x14)
#define GP202_IO_MUX_SLEEP_EN                  (GP202_PADCTRL_BASE0 + 0x48)
#define GP202_SLEEP_EN                         (GP202_PADCTRL_BASE0 + 0x18)
#define GP202_IO_MUX_PMIC_INT                  (GP202_PADCTRL_BASE0 + 0x4c)
#define GP202_PMIC_INT                         (GP202_PADCTRL_BASE0 + 0x1c)
#define GP202_IO_MUX_PMIC_RSTN                 (GP202_PADCTRL_BASE0 + 0x50)
#define GP202_PMIC_RSTN                        (GP202_PADCTRL_BASE0 + 0x20)
#define GP202_IO_MUX_BB_32K                    (GP202_PADCTRL_BASE0 + 0x54)
#define GP202_BB_32K                           (GP202_PADCTRL_BASE0 + 0x24)
#define GP202_IO_MUX_GPIO1                     (GP202_PADCTRL_BASE0 + 0x58)
#define GP202_GPIO1                            (GP202_PADCTRL_BASE0 + 0x28)
#define GP202_IO_MUX_GPIO2                     (GP202_PADCTRL_BASE0 + 0x5c)
#define GP202_GPIO2                            (GP202_PADCTRL_BASE0 + 0x2c)
#define GP202_IO_MUX_GPIO3                     (GP202_PADCTRL_BASE0 + 0x60)
#define GP202_GPIO3                            (GP202_PADCTRL_BASE0 + 0x30)
#define GP202_IO_MUX_CLK26M_EN0                (GP202_PADCTRL_BASE0 + 0x64)
#define GP202_CLK26M_EN0                       (GP202_PADCTRL_BASE0 + 0x34)
#define GP202_IO_MUX_RST_REQ                   (GP202_PADCTRL_BASE0 + 0x68)
#define GP202_RST_REQ                          (GP202_PADCTRL_BASE0 + 0x38)
#define GP202_IO_MUX_IIC_CLK                   (GP202_PADCTRL_BASE1 + 0x20)
#define GP202_IIC_CLK                          (GP202_PADCTRL_BASE1 + 0x10)
#define GP202_IO_MUX_IIC_DAT                   (GP202_PADCTRL_BASE1 + 0x24)
#define GP202_IIC_DAT                          (GP202_PADCTRL_BASE1 + 0x14)
#define GP202_IO_MUX_GPIO4                     (GP202_PADCTRL_BASE1 + 0x28)
#define GP202_GPIO4                            (GP202_PADCTRL_BASE1 + 0x18)

#define BIT_GP202_PAD_DRV(x)                   (((x) & 3))  //PMIC PAD driver strength select
#define BIT_GP202_PAD_WPU                      BIT_3        //PMIC PAD weak pull up
#define BIT_GP202_PAD_WPD                      BIT_2        //PMIC PAD weak pull down
#define BIT_GP202_PAD_PX                       0            //PMIC PAD no pull up and no pull down
#define BIT_GP202_PAD_SE                       BIT_4        //PMIC PAD schmitt trigger enable

#endif /*__PMIC_PINLIST_H__*/
