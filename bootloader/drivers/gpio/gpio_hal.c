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

#include <assert.h>
#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_type.h>
#include <console/console.h>
#include <gpio_cfg.h>
#include <gpio_inc.h>
#include <gpio_phy.h>

struct gpio_register_config {
    /* gpio callback function and param */
    GPIO_CBFUN          cb;
    void                *param;
    /* gpio direction type */
    GPIO_MODE           type;
};
static struct gpio_register_config g_gpio_config[GPIO_PIN_MAX];

static GPIO_IP _gpio_ip_get(GPIO_PIN_ID index)
{
    u32 i, pin_cnt = 0;

    for (i = 0; i < GPIO_IP_MAX; i++) {
        pin_cnt += gpio_bd_cfg[i].pin_cnt;
        if (index < pin_cnt) {
            return gpio_bd_cfg[i].index;
        }
    }

    ASSERT_INFO(0,"The gpio index was wrong, cannot find gpio ip!");
    return 0;
}

void aic_gpio_output(GPIO_PIN_ID index, GPIO_MODE mode)
{
    GPIO_IP ip;

    ASSERT((index < GPIO_PIN_MAX) && (mode <= GPIO_OUTPUT_LVL_HIGH));

    /* set gpio glb */
    ip = _gpio_ip_get(index);
    AIC_WRITE32(gpio_bd_cfg[ip].glb_base, gpio_bd_cfg[ip].glb_bit);

    g_gpio_config[index].type = mode;

    aic_gpio_phy_set_mode(index, mode);
}

u32 aic_gpio_level_get(GPIO_PIN_ID index)
{
    ASSERT((index < GPIO_PIN_MAX) && (g_gpio_config[index].type >= GPIO_TRIGGER_LVL_LOW));

    return aic_gpio_phy_get_value(index);
}

void aic_gpio_input(GPIO_PIN_ID index, GPIO_MODE mode, GPIO_CBFUN gpio_cb, void *param)
{
    GPIO_IP ip;

    /* the gpio pin must in input mode */
    ASSERT((index < GPIO_PIN_MAX) && (mode >= GPIO_TRIGGER_LVL_LOW));

    /* set gpio glb */
    ip = _gpio_ip_get(index);
    AIC_WRITE32(gpio_bd_cfg[ip].glb_base, gpio_bd_cfg[ip].glb_bit);

    g_gpio_config[index].cb    = gpio_cb;
    g_gpio_config[index].param = param;
    g_gpio_config[index].type  = mode;

    aic_gpio_phy_set_mode(index, mode);
}

int aic_gpio_request(GPIO_PIN_ID index)
{
    //Nothing todo

    return AIC_SUCCESS;
}

void aic_gpio_free(GPIO_PIN_ID index)
{
    //Nothing todo
}
