/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of pmic gpio module, it is pmic gpio
 *    phy driver header file.
 */

u32 aic_gpio_pmic_phy_pin_to_index(GPIO_PIN_ID ip_id, u32 pin_offset);
void aic_gpio_pmic_phy_set_mode(GPIO_PIN_ID index, GPIO_MODE mode);
u32 aic_gpio_pmic_phy_get_value(GPIO_PIN_ID index);
u32 aic_gpio_pmic_phy_get_int_state(GPIO_IP ip);
void aic_gpio_pmic_phy_int_clr(GPIO_PMIC_PIN_ID index);
void aic_gpio_pmic_phy_int_disable(GPIO_PIN_ID index);
void aic_gpio_pmic_phy_init(void);

