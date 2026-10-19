/*****************************************************************************/
/*                                                                           */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines gpo driver api.
 */

#ifndef __GPO_INC_H__
#define __GPO_INC_H__

typedef enum  {
    GPO_0 = 0,
    GPO_1,
    GPO_2,
    GPO_3,
    GPO_4,
    GPO_5,
    GPO_6,
    GPO_7,
    GPO_8,
    GPO_9,
    GPO_10,
    GPO_11,
    GPO_12,
    GPO_13,
    GPO_14,
    GPO_15,
    GPO_16,
    GPO_17,
    GPO_18,
    GPO_19,
    GPO_20,
    GPO_21,
    GPO_22,
    GPO_23,
    GPO_24,
    GPO_25,
    GPO_26,
    GPO_27,
    GPO_28,
    GPO_29,
    GPO_30,
    GPO_31,
    GPO_32,
    GPO_33,
    GPO_34,
    GPO_35,
    GPO_36,
    GPO_37,
    GPO_38,
    GPO_39,
    GPO_40,
    GPO_41,
    GPO_42,
    GPO_43,
    GPO_44,
    GPO_45,
    GPO_46,
    GPO_47,
    GPO_48,
    GPO_49,
    GPO_50,
    GPO_51,
    GPO_52,
    GPO_53,
    GPO_54,
    GPO_55,
    GPO_56,
    GPO_57,
    GPO_58,
    GPO_59,
    GPO_60,
    GPO_61,
    GPO_62,
    GPO_63,
    GPO_64,
    GPO_65,
    GPO_66,
    GPO_67,
    GPO_68,
    GPO_69,
    GPO_70,
    GPO_PIN_MAX
} GPO_PIN_ID;

typedef enum {
    GPO_OUTPUT_LOW,
    GPO_OUTPUT_HIGH,
    GPO_DISABLE,
} GPO_MODE;

/*
 * aic_gpo_output
 * Config pin as gpo mode, and output some level.
 * @pin: the pin which needs to be config as gpo mode, pin should be id number
 * @mode: the pin output mode
 */
void aic_gpo_output(u32 pin, GPO_MODE mode);

/*
 * aic_gpo_get_level
 * Use to get pin gpo mode
 * @pin: the pin which needs to be read, pin should be id number
 * return val: gpo level mode
 */
GPO_MODE aic_gpo_get_level(u32 pin);

/*
 * aic_gpo_pin_bare_cfg
 * It is special api, maybe the pin has not gpo id, only base addr.
 * @pin_base: the pin which needs to be config as gpo mode, pin_base should be
 * pin base address.
 * @mode: the pin output mode
 */
void aic_gpo_pin_bare_cfg(u32 pin_base, GPO_MODE mode);

#endif /* __GPO_INC_H__ */

