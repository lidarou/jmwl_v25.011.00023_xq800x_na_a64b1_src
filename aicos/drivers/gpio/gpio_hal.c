/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of gpio module, it is gpio
 *    common hal driver.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1539cc
#define LOG_MODULE    AMOD_GPIO

#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_type.h>
#include <aic_osa.h>
#include <core_op.h>

#include <gpio_inc.h>
#include <gpio_phy.h>

struct gpio_register_config g_gpio_config[GPIO_PIN_MAX];
static BOOL g_gpio_at_mode = FALSE;

GPIO_MODE aic_gpio_get_mode(GPIO_PIN_ID index)
{
    AIC_ASSERT(index < GPIO_PIN_MAX);

    return g_gpio_config[index].type;
}

/*
 * aic_gpio_output_raw
 * Configure some gpio as output high/low
 * Notice!!! Notice!!! Notice!!!
 * This is simple api, there is no assert judgement, no g_gpio_config set
 * and no pin configuration. It cannot keep the gpio level when chip sleep.
 */
void aic_gpio_output_raw(GPIO_PIN_ID index, GPIO_MODE mode)
{
    aic_gpio_phy_set_mode_raw(index, mode);
}

void aic_gpio_output(GPIO_PIN_ID index, GPIO_MODE mode)
{
    AIC_ASSERT((index < GPIO_PIN_MAX) && (mode <= GPIO_OUTPUT_LVL_HIGH));

    g_gpio_config[index].type = mode;
    g_gpio_config[index].used = TRUE;

    aic_gpio_phy_set_mode(index, mode);
}

u32 aic_gpio_level_get(GPIO_PIN_ID index)
{
    AIC_ASSERT(index < GPIO_PIN_MAX);

    return aic_gpio_phy_get_value(index);
}

void aic_gpio_input(GPIO_PIN_ID index, GPIO_MODE mode, GPIO_CBFUN gpio_cb, void *param)
{
    AIC_INT_SAVE_AREA
    /* the gpio pin must in input mode */
    AIC_ASSERT((index < GPIO_PIN_MAX) && (mode >= GPIO_TRIGGER_LVL_LOW));
    AIC_INT_DISABLE

    g_gpio_config[index].cb    = gpio_cb;
    g_gpio_config[index].param = param;
    g_gpio_config[index].type  = mode;
    g_gpio_config[index].used  = TRUE;
    AIC_INT_RESTORE

    aic_gpio_phy_set_mode(index, mode);
}

int aic_gpio_request(GPIO_PIN_ID index)
{
    //DO nothing
    return AIC_SUCCESS;
}

void aic_gpio_free(GPIO_PIN_ID index)
{
    //DO nothing
}

void aic_gpio_at_mode(void)
{
    g_gpio_at_mode = TRUE;
}

/*
 * aic_gpio_pulse_generate
 * Use gpio to generate some pulse
 * @index:  gpio index, use which gpio to generate
 * @pulse_mode: pulse direction, high level pulse or low level pulse
 * @duration: pulse duration time, unit is ms
 * @cnt: define the pulse count
 */
void aic_gpio_pulse_generate(GPIO_PIN_ID index, GPIO_MODE pulse_mode, u32 duration, u32 cnt)
{
    struct gpio_register_config *cfg = &g_gpio_config[index];
    u32 i;

    /* the gpio pin must in input mode */
    AIC_ASSERT((index < GPIO_PIN_MAX) && cnt && duration &&
                (cfg->type <= GPIO_OUTPUT_LVL_HIGH) &&
                (pulse_mode <= GPIO_OUTPUT_LVL_HIGH));

    for (i = 0; i < cnt; i++) {
        aic_gpio_output(index, pulse_mode);
        aos_sleep(duration);
        aic_gpio_output(index, !pulse_mode);
        aos_sleep(duration);
    }
}

/*
 * aic_gpio_def_pulse_generate
 * Use default gpio to generate some pulse
 * @pulse_mode: pulse direction, high level pulse or low level pulse
 * @duration: pulse duration time, unit is ms
 * @cnt: define the pulse count
 */
void aic_gpio_def_pulse_generate(GPIO_MODE pulse_mode, u32 duration, u32 cnt)
{
    GPIO_PIN_ID index = aic_gpio_phy_get_pulse_def();

    aic_gpio_output(index, !pulse_mode);

    aic_gpio_pulse_generate(index, pulse_mode, duration, cnt);
}

int aic_gpio_init(void)
{
    aic_gpio_phy_init();

    return 0;
}

