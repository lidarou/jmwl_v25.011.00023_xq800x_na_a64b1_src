/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines clock map.
 */

#ifndef __AIC_IOT_CLK_MAP_H__
#define __AIC_IOT_CLK_MAP_H__

/* gate clocks */
//#define CLK_TOP_SBUS_EB                                          0
//#define CLK_TOP_I2CC_EB                                          1
//#define CLK_TOP_I2C_PMIC_EB                                      2
//#define CLK_TOP_THMC_EB                                          3
//#define CLK_TOP_UART_EB                                          4
//#define CLK_TOP_IIS_EB                                           5
//#define CLK_TOP_UART_TX_EB                                       6
//#define CLK_TOP_SYSTMR_RF_EB                                     7
//#define CLK_TOP_MAILBOX_EB                                       8
//#define CLK_TOP_SPINLOCK_EB                                      9
//#define CLK_TOP_PLL_RF_EB                                        10
//#define CLK_TOP_RF_TOP_EB                                        11
//#define CLK_TOP_PMRO_EB                                          12
//#define CLK_TOP_EFUSE_EB                                         13
//#define CLK_TOP_PWM0_EB                                          14
//#define CLK_TOP_PWM1_EB                                          15
//#define CLK_TOP_GPIO_EB                                          16
//#define CLK_TOP_WDT_MCU_EB                                       17
//#define CLK_TOP_TRNG_EB                                          18
//#define CLK_PSRAMC_EB                                            19
//#define CLK_MEM_MAIN_EB                                          20
//#define CLK_AXI_CP2MEM_EB                                        21
//#define CLK_AXI_CPU2MEM_EB                                       22
//#define CLK_AXI_PERF_M0_EB                                       23
//#define CLK_AXI_PERF_M1_EB                                       24
//#define CLK_AXI_PERF_M2_EB                                       25
//#define CLK_NORFLASH_EB                                          26
//#define CLK_AUX0_EB                                              27
//#define CLK_AUX1_EB                                              28
//#define CLK_OCMC_EB                                              29
//#define CLK_CAM_MCLK_EB                                          30
//#define CLK_USB_REF_EB                                           31
//#define CLK_CAL_26M_EB                                           32
//#define CLK_TOP2CPU_DAP_EB                                       33
//#define CLK_RCO_MOD_EB                                           34
//#define CLK_LPUART_PD_EB                                         35
//#define CLK_AON_GLB_EB                                           36
//#define CLK_AON_IPU_CP_EB                                        37
//#define CLK_AON_APB_IPU_MCU_EB                                   38
//#define CLK_AON_SYSTMR_CNT_EB                                    39
//#define CLK_AON_EIPU_MCU_EB                                      40
//#define CLK_AON_DAP_EB                                           41
//#define CLK_CMSTAR_EB                                            42
//#define CLK_CMSTAR_DAP_EB                                        43
//#define CLK_MCU_MTX_MAIN0_EB                                     44
//#define CLK_MCU_LZMA_EB                                          45
//#define CLK_MCU_MTX_S3_EB                                        46
//#define CLK_MCU_MTB_EB                                           47
//#define CLK_ROMCODE_EB                                           48
//#define CLK_MCU_MTX_MAIN1_EB                                     49
//#define CLK_MCU_DMA_EB                                           50
//#define CLK_MCU_PPPE_EB                                          51
//#define CLK_MCU_PPPD_EB                                          52
//#define CLK_MCU_USB_EB                                           53
//#define CLK_MCU_LCDC_EB                                          54
//#define CLK_MCU_CSI_EB                                           55
//#define CLK_MCU_MTX_S2_EB                                        56
//#define CLK_MCU_AHB2AHB_EB                                       57
//#define CLK_MCU_AHB2APB_EB                                       58
//#define CLK_MCU_I2C0_EB                                          59
//#define CLK_MCU_I2C1_EB                                          60
//#define CLK_MCU_UART0_EB                                         61
//#define CLK_MCU_UART1_EB                                         62
//#define CLK_MCU_SSP0_EB                                          63
//#define CLK_MCU_CAN_EB                                           64
//#define CLK_MCU_TMR_EB                                           65
//#define CLK_CR5_PCDUMP_EB                                        66
//#define CLK_MCU_CAN_TMR_EB                                       67
//#define CLK_CPU_MTX_EB                                           68
//#define CLK_MCU_MTX_S0_EB                                        69
//#define CLK_MCU_PTEST_EB                                         70
//#define CLK_CPU_DAP_TPI_EB                                       71
//#define CLK_CPU_TPI_EB                                           72
//#define CLK_ND_EIPU_EB                                           73
//#define CLK_ND_UART_RX_EB                                        74
//#define CLK_NON_ND_UART_RX_EB                                    75
//#define CLK_ND_TMR_MCU_EB                                        76
//#define CLK_PMU_32K_EB                                           77
//#define CLK_AON_SBUS_EB                                          78
//#define CLK_ND_PWR_EB                                            79

/* pll gate clocks  */
#define CLK_DPLL1_EN                                               0
#define CLK_CAM_MCLK_EB                                            1

/* div clocks */
//#define CLK_TOP_WDT_MCU_CFG                                         0
//#define CLK_TOP_TRNG_CFG                                            1
//#define CLK_MCU_CAN_CFG                                             2

/* mux clocks */
#define CLK_TOP_IIS_CFG                                             0
#define CLK_OCMC_CFG                                                1
#define CLK_CAM_MCLK_CFG                                            2
#define CLK_USB_REF_CFG                                             3
#define CLK_RCO_CFG                                                 4
#define CLK_TOP_LPUART_PD_CFG                                       5
//#define CLK_MCU_TMR_CFG                                             6
#define CLK_ND_UART_RX                                              6

/* composite clocks */
//#define CLK_TOP_APB_SBUS_CFG                                        0
//#define CLK_TOP_EFUSE_CFG                                           1
//#define CLK_TOP_PWM0_CFG                                            2
//#define CLK_TOP_PWM1_CFG                                            3
//#define CLK_TOP_GPIO_CFG                                            4
#define CLK_TOP_UART_CFG                                            0
#define CLK_PSRAMC_2X_CFG                                           1
#define CLK_REF_NORFLASH_CFG                                        2
//#define CLK_AUX0_CFG                                                8
//#define CLK_AUX1_CFG                                                9
//#define CLK_TOP2CPU_DAP_CFG                                         10
//#define CLK_AON_DAP_CFG                                             11
#define CLK_MCU_CMSTAR_CFG                                          3
#define CLK_MCU_UART0_CFG                                           4
#define CLK_MCU_UART1_CFG                                           5
#define CLK_MCU_SSP0_CFG                                            6
#define CLK_MCU_LCDC_CFG                                            7
/*#define CLK_MCU_CAN_TMR_CFG                                         17
#define CLK_MCU_PPPD_CFG                                            18
#define CLK_MCU_PPPE_CFG                                            19*/

#endif /* __AIC_IOT_CLK_MAP_H__ */
