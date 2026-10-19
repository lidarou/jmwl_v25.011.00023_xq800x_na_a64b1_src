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
    GPIO_0 = 0,
    GPIO_1,
    GPIO_2,
    GPIO_3,
    GPIO_4,
    GPIO_5,
    GPIO_6,
    GPIO_7,
    GPIO_8,
    GPIO_9,
    GPIO_10,
    GPIO_11,
    GPIO_12,
    GPIO_13,
    GPIO_14,
    GPIO_15,
    GPIO_16,
    GPIO_17,
    GPIO_18,
    GPIO_19,
    GPIO_20,
    GPIO_21,
    GPIO_22,
    GPIO_23,
    GPIO_24,
    GPIO_25,
    GPIO_26,
    GPIO_27,
    GPIO_28,
    GPIO_29,
    GPIO_30,
    GPIO_31,
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
