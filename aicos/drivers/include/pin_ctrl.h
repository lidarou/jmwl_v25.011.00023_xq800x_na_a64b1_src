/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   Header file of pin control.
*/
#ifndef __PIN_CTRL_H__
#define __PIN_CTRL_H__

/**
 * pmic_gpio9_f0tof2() - Change pmic gpio9 function from 0 to 2
 *
 * For gx316, the default function of pmic gpio9 is 0(gpio). It should be
 * changed to 2 (adc) when touch screen controller enabled.
 *
 * Return: AIC_SUCCESS or errno.
 */
s32 pmic_gpio9_f0tof2(void);

/**
 * pin_gpio32_f3tof6() - Change pin SDMMC_RST/gpio32 function from 3 to 6
 *
 * For gx316, the default function of pin SDMMC_RST/gpio32 is 6(EIU_INT0).
 * It should bechanged to 6 (EIU_INT0).
 */
void pin_eiu_int0_f3tof6(void);

/**
 * pin_gpio32_f3tof6() - Change pin SDMMC_RST/gpio32 function from 6 to 3
 *
 * For gx316, the default function of pin SDMMC_RST/gpio32 is 6(EIU_INT0).
 * It should bechanged to 3 (gpio).
 */
void pin_eiu_int0_f6tof3(void);

/**
 * pin_func_config() - Change pin function
 *
 * @pin_reg: pin register address
 * @func: pin function, only support 0 ~ 6
 */
void pin_func_config(u32 pin_reg, u32 func);

#endif /* __PIN_CTRL_H__ */
