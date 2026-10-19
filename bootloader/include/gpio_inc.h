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

#include <aic_type.h>
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
    GPIO_INPUT_NOINT
} GPIO_MODE;

void aic_gpio_output(GPIO_PIN_ID index, GPIO_MODE mode);
u32 aic_gpio_level_get(GPIO_PIN_ID index);
void aic_gpio_input(GPIO_PIN_ID index, GPIO_MODE mode, GPIO_CBFUN gpio_cb, void *param);
int aic_gpio_request(GPIO_PIN_ID index);
void aic_gpio_free(GPIO_PIN_ID index);

#endif /* __GPIO_INC_H__ */

