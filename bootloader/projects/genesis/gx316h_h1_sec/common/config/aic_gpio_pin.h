/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file includes chip ap dma hw request channels info.
 */


#ifndef __AIC_GPIO_PIN_H__
#define __AIC_GPIO_PIN_H__

typedef enum  {
    GPIO_AON_0 = 0,
    GPIO_AON_1,
    GPIO_AON_2,
    GPIO_AON_3,
    GPIO_AON_4,
    GPIO_AON_5,
    GPIO_AON_6,
    GPIO_AON_7,
    GPIO_AON_8,
    GPIO_AON_9,
    GPIO_AON_10,
    GPIO_AON_11,
    GPIO_AON_12,
    GPIO_AON_13,
    GPIO_AON_14,
    GPIO_AON_15,
    GPIO_TOP_0 = 16,
    GPIO_TOP_1,
    GPIO_TOP_2,
    GPIO_TOP_3,
    GPIO_TOP_4,
    GPIO_TOP_5,
    GPIO_TOP_6,
    GPIO_TOP_7,
    GPIO_TOP_8,
    GPIO_TOP_9,
    GPIO_TOP_10,
    GPIO_TOP_11,
    GPIO_TOP_12,
    GPIO_TOP_13,
    GPIO_TOP_14,
    GPIO_TOP_15,
    GPIO_TOP_16,
    GPIO_TOP_17,
    GPIO_TOP_18,
    GPIO_TOP_19,
    GPIO_TOP_20,
    GPIO_TOP_21,
    GPIO_TOP_22,
    GPIO_TOP_23,
    GPIO_TOP_24,
    GPIO_TOP_25,
    GPIO_TOP_26,
    GPIO_TOP_27,
    GPIO_TOP_28,
    GPIO_TOP_29,
    GPIO_TOP_30,
    GPIO_TOP_31,
    GPIO_PIN_MAX
} GPIO_PIN_ID;

typedef enum  {
    GPIO_AON,
    GPIO_TOP,
    GPIO_IP_MAX
} GPIO_IP;

typedef enum  {
    GPIO_PMIC_AON_0 = 0,
    GPIO_PMIC_AON_1,
    GPIO_PMIC_AON_2,
    GPIO_PMIC_PD_0 = 3,
    GPIO_PMIC_PD_1,
    GPIO_PMIC_PD_2,
    GPIO_PMIC_PD_3,
    GPIO_PMIC_PD_4,
    GPIO_PMIC_PD_5,
    GPIO_PMIC_PD_6,
    GPIO_PMIC_PD_7,
    GPIO_PMIC_PD_8,
    GPIO_PMIC_PIN_MAX
} GPIO_PMIC_PIN_ID;

typedef enum  {
    GPIO_PMIC_AON,
    GPIO_PMIC_PD,
    GPIO_PMIC_MAX
} GPIO_PMIC_IP;

#endif /* __AIC_GPIO_PIN_H__ */
