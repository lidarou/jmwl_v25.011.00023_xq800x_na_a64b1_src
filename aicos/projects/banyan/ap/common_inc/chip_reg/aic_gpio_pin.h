/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file includes chip gpio pin info.
 */


#ifndef __AIC_GPIO_PIN_H__
#define __AIC_GPIO_PIN_H__

#define AIC_GPIO_GPO_MAPPING_ID         100

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
    /* GPIO GPO index, phy gpo index in gpo_cfg.h */
    GPIO_GPO_0  = AIC_GPIO_GPO_MAPPING_ID,
    GPIO_GPO_1,
    GPIO_GPO_2,
    GPIO_GPO_3,
    GPIO_GPO_4,
    GPIO_GPO_5,
    GPIO_GPO_6,
    GPIO_GPO_7,
    GPIO_GPO_8,
    GPIO_GPO_9,
    GPIO_GPO_10,
    GPIO_GPO_11,
    GPIO_GPO_12,
    GPIO_GPO_13,
    GPIO_GPO_14,
    GPIO_GPO_15,
    GPIO_GPO_16,
    GPIO_GPO_17,
    GPIO_GPO_18,
    GPIO_GPO_19,
    GPIO_GPO_20,
    GPIO_GPO_21,
    GPIO_GPO_22,
    GPIO_GPO_23,
    GPIO_GPO_24,
    GPIO_GPO_25,
    GPIO_GPO_26,
    GPIO_GPO_27,
    GPIO_GPO_28,
    GPIO_GPO_29,
    GPIO_GPO_30,
    GPIO_GPO_31,
    GPIO_GPO_32,
    GPIO_GPO_33,
    GPIO_GPO_34,
    GPIO_GPO_35,
    GPIO_GPO_36,
    GPIO_GPO_37,
    GPIO_GPO_38,
    GPIO_GPO_39,
    GPIO_GPO_40,
    GPIO_GPO_41,
    GPIO_GPO_42,
    GPIO_GPO_43,
    GPIO_GPO_44,
    GPIO_GPO_45,
    GPIO_GPO_46,
    GPIO_GPO_47,
    GPIO_GPO_48,
    GPIO_GPO_49,
    GPIO_GPO_50,
    GPIO_GPO_51,
    GPIO_GPO_52,
    GPIO_GPO_53,
    GPIO_GPO_54,
    GPIO_GPO_55,
    GPIO_GPO_56,
    GPIO_GPO_57,
    GPIO_GPO_58,
    GPIO_GPO_59,
    GPIO_GPO_60,
    GPIO_PIN_MAX
} GPIO_PIN_ID;

typedef enum  {
    GPIO_AON,
    GPIO_TOP,
    GPIO_IP_MAX
} GPIO_IP;

typedef enum  {
    GPIO_PMIC_0 = 0, //AON
    GPIO_PMIC_1,     //AON
    GPIO_PMIC_2,     //AON
    GPIO_PMIC_3,     //AON
    GPIO_PMIC_4,     //PD
    GPIO_PMIC_PIN_MAX
} GPIO_PMIC_PIN_ID;

typedef enum  {
    GPIO_PMIC_AON,
    GPIO_PMIC_PD,
    GPIO_PMIC_MAX
} GPIO_PMIC_IP;

#endif /* __AIC_GPIO_PIN_H__ */
