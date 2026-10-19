/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of pmic gpio module, it is pmic gpio
 *    common hal driver.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x111462
#define LOG_MODULE    AMOD_PMIC

#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_type.h>
#include <core_op.h>
#include <irq.h>

#include <gpio_inc.h>
#include "gpio_interface.h"

struct gpio_register_config g_gpio_pmic_config[GPIO_PMIC_PIN_MAX];

void aic_gpio_pmic_output(GPIO_PMIC_PIN_ID index, GPIO_MODE mode)
{
    AIC_ASSERT((index < GPIO_PMIC_PIN_MAX) && (mode <= GPIO_OUTPUT_LVL_HIGH));

    g_gpio_pmic_config[index].type = mode;
    g_gpio_pmic_config[index].used = TRUE;

    aic_gpio_pmic_phy_set_mode(index, mode);
}

u32 aic_gpio_pmic_level_get(GPIO_PMIC_PIN_ID index)
{
    AIC_ASSERT(index < GPIO_PMIC_PIN_MAX);

    return aic_gpio_pmic_phy_get_value(index);
}

void aic_gpio_pmic_input(GPIO_PMIC_PIN_ID index, GPIO_MODE mode, GPIO_CBFUN gpio_cb, void *param)
{
    AIC_INT_SAVE_AREA
    /* the gpio pin must in input mode */
    AIC_ASSERT((index < GPIO_PMIC_PIN_MAX) && (mode >= GPIO_TRIGGER_LVL_LOW));
    AIC_INT_DISABLE

    g_gpio_pmic_config[index].cb    = gpio_cb;
    g_gpio_pmic_config[index].param = param;
    g_gpio_pmic_config[index].type  = mode;
    g_gpio_pmic_config[index].used  = TRUE;
    AIC_INT_RESTORE

    aic_gpio_pmic_phy_set_mode(index, mode);
}

int aic_gpio_pmic_request(GPIO_PMIC_PIN_ID index)
{
    //Do nothing
    return AIC_SUCCESS;
}

void aic_gpio_pmic_free(GPIO_PMIC_PIN_ID index)
{
    //Do nothing
}

int aic_gpio_pmic_init(void)
{
    aic_gpio_pmic_phy_init();

    return 0;
}

AIC_DRV_L2_INIT_EXPORT(aic_gpio_pmic_init);

