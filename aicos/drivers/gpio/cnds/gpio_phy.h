/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of gpio module, it is gpio
 *    phy driver header file.
 */

u32 aic_gpio_phy_pin_to_index(GPIO_PIN_ID ip_id, u32 pin_offset);
void aic_gpio_phy_set_mode_raw(GPIO_PIN_ID index, GPIO_MODE mode);
void aic_gpio_phy_set_mode(GPIO_PIN_ID index, GPIO_MODE mode);
GPIO_MODE aic_gpio_phy_get_mode(GPIO_PIN_ID index);
u32 aic_gpio_phy_get_value(GPIO_PIN_ID index);
u32 aic_gpio_phy_get_int_state(GPIO_IP ip);
void aic_gpio_phy_int_disable(GPIO_PIN_ID index);
u32 aic_gpio_phy_get_pulse_def(void);
void aic_gpio_phy_init(void);

