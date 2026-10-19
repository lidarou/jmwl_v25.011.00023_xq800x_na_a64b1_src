/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines gpio driver api.
 */

#ifndef __GPIO_INC_H__
#define __GPIO_INC_H__

#include <aic_gpio_pin.h>

typedef void (*GPIO_CBFUN)(void *param);

typedef enum {
    GPIO_OUTPUT_LVL_LOW,
    GPIO_OUTPUT_LVL_HIGH,
    GPIO_TRIGGER_LVL_LOW,
    GPIO_TRIGGER_LVL_HIGH,
    GPIO_TRIGGER_EDGE_RISING,
    GPIO_TRIGGER_EDGE_FALLING,
    GPIO_TRIGGER_EDGE_ANY,
    GPIO_BYPASS,
    GPIO_INPUT_NOINT,
    GPIO_OUTPUT_NONE
} GPIO_MODE;

struct gpio_register_config {
    /* gpio callback function and param */
    GPIO_CBFUN          cb;
    void                *param;
    /* gpio direction type */
    GPIO_MODE           type;
    /* 0 - unused, 1 - used */
    volatile bool       used;

};

GPIO_MODE aic_gpio_get_mode(GPIO_PIN_ID index);
/*
 * aic_gpio_output_raw
 * Configure some gpio as output high/low
 * Notice!!! Notice!!! Notice!!!
 * This is simple api, there is no assert judgement, no g_gpio_config set
 * and no pin configuration. It cannot keep the gpio level when chip sleep.
 */
void aic_gpio_output_raw(GPIO_PIN_ID index, GPIO_MODE mode);
void aic_gpio_output(GPIO_PIN_ID index, GPIO_MODE mode);
u32 aic_gpio_level_get(GPIO_PIN_ID index);
void aic_gpio_input(GPIO_PIN_ID index, GPIO_MODE mode, GPIO_CBFUN gpio_cb, void *param);
int aic_gpio_request(GPIO_PIN_ID index);
void aic_gpio_free(GPIO_PIN_ID index);
/* AT mode, do not disable gpio clock */
void aic_gpio_at_mode(void);

/*
 * aic_gpio_pulse_config
 * Use gpio to generate some pulse
 * @index:  gpio index, use which gpio to generate
 * @pulse_mode: pulse direction, high level pulse or low level pulse
 * @duration: pulse duration time, unit is ms
 * @cnt: define the pulse count
 */
void aic_gpio_pulse_generate(GPIO_PIN_ID index, GPIO_MODE pulse_mode, u32 duration, u32 cnt);
/*
 * aic_gpio_def_pulse_generate
 * Use default gpio to generate some pulse
 * @pulse_mode: pulse direction, high level pulse or low level pulse
 * @duration: pulse duration time, unit is ms
 * @cnt: define the pulse count
 */
void aic_gpio_def_pulse_generate(GPIO_MODE pulse_mode, u32 duration, u32 cnt);
#ifdef CONFIG_PMIC_GPIO
int aic_gpio_pmic_request(GPIO_PMIC_PIN_ID index);
void aic_gpio_pmic_free(GPIO_PMIC_PIN_ID index);
void aic_gpio_pmic_output(GPIO_PMIC_PIN_ID index, GPIO_MODE mode);
void aic_gpio_pmic_input(GPIO_PMIC_PIN_ID index, GPIO_MODE mode, GPIO_CBFUN gpio_cb, void *param);
#endif

#endif /* __GPIO_INC_H__ */

