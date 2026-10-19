/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file includes chip interrupt list file.
 */

#ifndef __PMIC_INTERRUPT_H__
#define __PMIC_INTERRUPT_H__

typedef enum PMIC_IRQn
{
/* -------------------  PMIC Processor Interrupt Numbers  ------------------------------ */
    PMIC_IRQ_RTC                    =   0,
    PMIC_IRQ_UPS_CNT                =   1,
    PMIC_IRQ_EFUSE                  =   2,
    PMIC_IRQ_GPIO_UPS               =   3,
    PMIC_IRQ_GP202_GPIO_UPS         =   9,
    PMIC_IRQ_GPIO_PD                =   4,
    PMIC_IRQ_IIC2APB                =   5,
    PMIC_IRQ_WDT                    =   6,
    PMIC_IRQ_TSC                    =   7,
    PMIC_IRQ_ADC                    =   8,
    PMIC_IRQ_PMU_RTC_INT            =   9,
    PMIC_IRQ_PMU_UPS_INT            =   10,
    PMIC_IRQ_PMU_PD_INT             =   11,
    PMIC_IRQ_IIC_PSLERROR           =   12,
    PMIC_RCO_CALIBRE_TIMEOUT        =   13,
    PMIC_2ND_INT_START              =   14,
    PMIC_PMU_RTC_CHG_RISE           =   PMIC_2ND_INT_START,
    PMIC_PMU_RTC_CHG_FALL           =   15,
    PMIC_PMU_UPS_PWRKEY_PRESS       =   16,
    PMIC_PMU_UPS_RST_REQ            =   17,
    PMIC_PMU_UPS_RSTIN_N            =   18,
    PMIC_PMU_UPS_IIC_ACCESS         =   19,
    PMIC_PMU_PD_BUCK0_OC            =   20,
    PMIC_PMU_PD_BUCK1_OC            =   21,
    PMIC_PMU_PD_BUCK2_OC            =   22,
    PMIC_PMU_PD_IDET                =   23,
    PMIC_PMU_PD_CHRG_OVER           =   24,
    PMIC_PMU_PD_ERROR_VBAT          =   25,
    PMIC_PMU_PD_OT_DET1             =   26,
    PMIC_PMU_PD_OT_DET2             =   27,
    PMIC_PMU_IRQ_MAXIUM,
    PMIC_INT_MAXIUM = PMIC_PMU_IRQ_MAXIUM,
} PMIC_IRQn_Type;

#endif /* __PMIC_INTERRUPT_H__ */
