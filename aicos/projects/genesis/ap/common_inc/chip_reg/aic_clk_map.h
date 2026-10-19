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
#define CLK_AON_MTX_MAIN_EN        0
#define CLK_AON_MTX_M1_EN          1
#define CLK_AON_MTX_M2_EN          2
#define CLK_AON_AHB_MTX_S0_EN      3
#define CLK_TOP_APB_CRU_RF_EN      4
#define CLK_TOP_APB_CLK_EN_RF_EN   5
#define CLK_AON2CPU_MTX_EN         6
#define CLK_TOP2CPU_EIP2EFUSE_EN   7
#define CLK_AON_SYSTMR_EN          8
#define CLK_AON_TMR0_EN            9
#define CLK_AON_TMR1_EN            10
#define CLK_AON_TMR2_EN            11
#define CLK_AON_WDT0_EN            12
#define CLK_AON_WDT1_EN            13
#define CLK_AON_EIPU_EN            14
#define CLK_AON_EIPU2TOP_EN        15
#define CLK_AON_EIPU_SEC_EN        16
#define CLK_AON_TMR_SEC_EN         17
#define CLK_AON_WDT_SEC_EN         18
#define CLK_AON_GPD_EN             19
#define CLK_TOP_PWM0_EN            20
#define CLK_TOP_PWM1_EN            21
#define CLK_TOP_PWM2_EN            22
#define CLK_TOP_PWM3_EN            23
#define CLK_TOP_PWM4_EN            24
#define CLK_TOP_PWM5_EN            25
#define CLK_TOP_PWM6_EN            26
#define CLK_TOP_PWM7_EN            27
#define CLK_TOP_UART0_EN           28
#define CLK_TOP_UART1_EN           29
#define CLK_TOP_AON_UART_EN        30
#define CLK_AON_KPC_EN             31
#define CLK_TOP_MIPI_RFFE_EN       32
#define CLK_TOP_IIS0_EN            33
#define CLK_TOP_IIS1_EN            34
#define CLK_PSRAMC_2X_EN           35
#define CLK_PSRAMC_EN              36
#define CLK_PSRAMPHY_AT_SPEED_EN   37
#define CLK_AXI_CPU2MEM_EN         38
#define CLK_REF_NORFLASH_EN        39
#define CLK_REF_NORFLASH_DIV2_EN   40
#define CLK_AON_PWR_EN             41
#define CLK_AON_PMU_26M_EN         42
#define CLK_TOP_SDIOPHY_EN         43
#define CLK_AON_DAP_EN             44
#define CLK_AUX0_EN                45
#define CLK_AUX1_EN                46
#define CLK_AUX2_EN                47
#define CLK_ZJTAG_CTRL_TCK_EN      48
#define CLK_I_THERM_EN             49
#define CLK_OCMC_EN                50
#define CLK_26M_PSRAMC_EN          51
#define CLK_32K_PMU_EN             52
#define CLK_MCU_CMSTAR_EN          53
#define CLK_MCU_CMSTAR_DAP_EN      54
#define CLK_MCU_MTX_S5_EN          55
#define CLK_MCU_MTX_M1_EN          56
#define CLK_MCU_MTX_M0_EN          57
#define CLK_MCU_MTX_MAIN_EN        58
#define CLK_MCU_MTX_M2_EN          59
#define CLK_MCU_AXI_DMA_EN         60
#define CLK_MCU_MTX_M8_EN          61
#define CLK_MCU_MTX_M9_EN          62
#define CLK_MCU_AXI_PPP_E_EN       63
#define CLK_MCU_AXI_PPP_D_EN       64
#define CLK_MCU_MTX_M4_EN          65
#define CLK_MCU_AXI_USB_EN         66
#define CLK_MCU_MTX_S4_EN          67
#define CLK_MCU_MTX_M6_EN          68
#define CLK_MCU_AHB_LCDC_EN        69
#define CLK_MCU_MTX_M7_EN          70
#define CLK_MCU_AHB_CSI_EN         71
#define CLK_MCU_MTX_S1_EN          72
#define CLK_MCU_AHB_GLB_EN         73
#define CLK_MCU_AHB_LPC_EN         74
#define CLK_MCU_AHB_ROMCODE_EN     75
#define CLK_MCU_AHB_EIP_EN         76
#define CLK_MCU_MTX_S3_EN          77
#define CLK_MERGE_MTX_M4_EN        78
#define CLK_MCU_MTX_S2_EN          79
#define CLK_MCU_AHB2APB_EN         80
#define CLK_MCU_APB_I2C0_EN        81
#define CLK_MCU_APB_I2C1_EN        82
#define CLK_MCU_APB_UART0_EN       83
#define CLK_MCU_APB_UART1_EN       84
#define CLK_MCU_APB_UART2_EN       85
#define CLK_MCU_APB_TMR_EN         86
#define CLK_MCU_APB_SDIOH_EN       87
#define CLK_MCU_APB_SSP0_EN        88
#define CLK_MCU_APB_SSP1_EN        89
#define CLK_MCU_APB_PPP_D_EN       90
#define CLK_MCU_APB_PPP_E_EN       91
#define CLK_MCU_APB_LCDC_EN        92
#define CLK_MCU_APB_CSI_EN         93
#define CLK_CP_APB_PCDUMP_EN       94
#define CLK_MCU_UART0_EN           95
#define CLK_MCU_UART1_EN           96
#define CLK_MCU_UART2_EN           97
#define CLK_MCU_SSP0_EN            98
#define CLK_MCU_SSP1_EN            99
#define CLK_MCU_LCDC_EN            100
#define CLK_MCU_EIP_EN             101
#define CLK_MCU_MTX_M3_EN          102
#define CLK_MCU_EIP_CTR_EN         103
#define CLK_MCU_SDIOH_EN           104
#define CLK_MCU_TMR_EN             105
#define CLK_MCU_PPP_D_EN           106
#define CLK_MCU_PPP_E_EN           107
#define CLK_MERGE_MTX_S3_EN        108
#define CLK_MCU_MTX_M10_EN         109
#define CLK_MERGE_MTX_MAIN_MEM_EN  110
#define CLK_MCU_MTX_S0_EN          111
#define CLK_AUDIO_MTX_S0_EN        112
#define CLK_MERGE_MTX_M0_EN        113
#define CLK_MERGE_MTX_M1_EN        114
#define CLK_MERGE_MTX_S0_EN        115
#define CLK_MERGE_MTX_MAIN_PER_EN  116
#define CLK_MERGE_MTX_S5_EN        117
#define CLK_AUDIO_MTX_M3_EN        118
#define CLK_MERGE_MTX_S1_EN        119
#define CLK_MERGE_MTX_M5_EN        120
#define CLK_MERGE_MTX_MAIN_DBG_EN  121
#define CLK_MCU_SDPHY_REG_PCLK_EN  122
#define CLK_MCU_MTX_M5_EN          123
#define CLK_MERGE_MTX_S2_EN        124
#define CLK_CRU_MCU_RF_EN          125
#define CLK_CRU_AUDIO_RF_EN        126
#define CLK_CRU_CP_RF_EN           127
#define CLK_CKM_EN_MCU_RF_EN       128
#define CLK_CKM_EN_AUDIO_RF_EN     129
#define CLK_CKM_EN_CP_RF_EN        130
#define CLK_CPU_AHB_LPC_EN         131
#define CLK_CPU_AHB_LZMA_EN        132
#define CLK_CPU_GLB_EN             133
#define CLK_MCU_UTMI_EN            134
#define CLK_MCU_USB_PTEST_EN       135
#define CLK_MCU_PTEST_EN           136
#define CLK_EIP2EFUSE_EN           137
#define CLK_MCU_LZMA_EN            138
#define CLK_MERGE_MTX_M6_EN        139
#define CLK_CPU_DAP_EN             140
#define CLK_CPU_ZJTAG_TCK_EN       141
#define CLK_BLK_CP_CORE_EN         142
#define CLK_BLK_DSP_CORE_EN        143

/* pll gate clocks  */
#define CLK_UPLL_EN                0
#define CLK_UPLL_24M_EN            1
#define CLK_APLL_EN                2

/* div clocks */
#define CLK_AON_PMU_26M            0
#define CLK_AON_SIM0               1
#define CLK_ZJTAG_CTRL_TCK         2
#define CLK_26M_PSRAMC             3

/* mux clocks */
#define CLK_OCMC                   0
#define CLK_MCU_EIP_CTR            1

/* composite clocks */
#define CLK_AON_MTX_MAIN           0
#define CLK_AON_GPD                1
#define CLK_TOP_PWM0               2
#define CLK_AON_KPC                3
#define CLK_TOP_MIPI_RFFE          4
#define CLK_TOP_IIS0               5
#define CLK_PSRAMC_2X              6
#define CLK_REF_NORFLASH           7
#define CLK_TOP_SDIOPHY            8
#define CLK_AON_DAP                9
#define CLK_AUX0                   10
#define CLK_AUX1                   11
#define CLK_AUX2                   12
#define CLK_MCU_CMSTAR             13
#define CLK_MCU_AHB2APB            14
#define CLK_MCU_UART0              15
#define CLK_MCU_UART1              16
#define CLK_MCU_UART2              17
#define CLK_MCU_SSP0               18
#define CLK_MCU_SSP1               19
#define CLK_MCU_LCDC               20
#define CLK_MCU_EIP                21
#define CLK_MCU_TMR                22
#define CLK_MCU_PPP_D              23
#define CLK_MCU_PPP_E              24
#define CLK_MCU_MERGE_MTX_S2       25
#define CLK_MCU_USB_PTEST          26
#define CLK_MCU_PTEST              27
#define CLK_MCU_LZMA               28
#define CLK_BLK_CP_CORE            29
#define CLK_BLK_DSP_CORE           30

#endif /* __AIC_IOT_CLK_MAP_H__ */
