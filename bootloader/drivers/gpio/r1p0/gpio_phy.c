/*****************************************************************************/
/*                                                                           */
/*   Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the software driver of gpio module.
 */

#include <assert.h>
#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <console/console.h>
#include <gpio_cfg.h>
#include <gpio_inc.h>

/* GPIO registers define */
#define GPIO_BYPASS_EN      0x0
#define GPIO_DIRECTION      0x4
#define GPIO_OUTPUT_EN      0x8
#define GPIO_OUTPUT_VAL     0xC
#define GPIO_INTPUT_VAL     0x10
#define GPIO_INT_EN         0x14
#define GPIO_INT_MASK       0x18
#define GPIO_INT_RAW        0x1C
#define GPIO_INT_STATUS     0x20
#define GPIO_INT_CLR        0x24
#define GPIO_INPUT_TYPE     0x28
#define GPIO_INPUT_VAL      0x2C
#define GPIO_INPUT_ON_ANY   0x30

static GPIO_IP _gpio_ip_get(GPIO_PIN_ID index)
{
    u32 ip_max = sizeof(gpio_bd_cfg) / sizeof(*gpio_bd_cfg);
    u32 i, pin_cnt = 0;

    for (i = 0; i < ip_max; i++) {
        pin_cnt += gpio_bd_cfg[i].pin_cnt;
        if (index < pin_cnt) {
            return i;
        }
    }

    ASSERT_INFO(0,"The gpio index was wrong, cannot find gpio ip!");
    return 0;
}

static u32 _gpio_index_to_pin(GPIO_PIN_ID index)
{
    u32 ip_max = sizeof(gpio_bd_cfg) / sizeof(*gpio_bd_cfg);
    u32 i, pin_cnt = 0;

    for (i = 0; i < ip_max; i++) {
        pin_cnt += gpio_bd_cfg[i].pin_cnt;
        /* why -1? becasue pin index is start from 0 */
        if (index > pin_cnt - 1) {
            continue;
        } else {
            return BIT(index + gpio_bd_cfg[i].pin_cnt - pin_cnt);
        }
    }

    ASSERT_INFO(0,"The gpio pin was wrong, cannot find gpio pin!");
    return 0;
}

u32 aic_gpio_phy_pin_to_index(GPIO_PIN_ID ip_id, u32 pin_offset)
{
    u32 ip_max = sizeof(gpio_bd_cfg) / sizeof(*gpio_bd_cfg);
    u32 i, pin_index = pin_offset;

    for (i = 0; i < ip_max; i++) {
        if (ip_id > i) {
            pin_index += gpio_bd_cfg[i].pin_cnt;
        } else {
            return pin_index;
        }
    }

    ASSERT_INFO(0,"The gpio param was wrong, cannot find gpio pin index!");
    return 0;
}

void aic_gpio_phy_set_mode(GPIO_PIN_ID index, GPIO_MODE mode)
{
    GPIO_IP ip = _gpio_ip_get(index);
    u32 base, pin_bit;

    pin_bit = _gpio_index_to_pin(index);
    base    = gpio_bd_cfg[ip].base;

    switch (mode) {
    case GPIO_OUTPUT_LVL_LOW:
        /* set pin as output mode */
        AIC_REG_CLRVAL(base + GPIO_DIRECTION, pin_bit);
        /* set pin as output enable */
        AIC_REG_SETVAL(base + GPIO_OUTPUT_EN, pin_bit);
        /* set pin as output low */
        AIC_REG_CLRVAL(base + GPIO_OUTPUT_VAL, pin_bit);
        break;
    case GPIO_OUTPUT_LVL_HIGH:
        /* set pin as output mode */
        AIC_REG_CLRVAL(base + GPIO_DIRECTION, pin_bit);
        /* set pin as output enable */
        AIC_REG_SETVAL(base + GPIO_OUTPUT_EN, pin_bit);
        /* set pin as output high */
        AIC_REG_SETVAL(base + GPIO_OUTPUT_VAL, pin_bit);
        break;
    case GPIO_TRIGGER_LVL_LOW:
        /* disable output enable in input mode */
        AIC_REG_CLRVAL(base + GPIO_OUTPUT_EN, pin_bit);
        /* set trigger type as low level mode */
        AIC_REG_CLRVAL(base + GPIO_INPUT_VAL, pin_bit);
        /* set trigger type as level triggering mode */
        AIC_REG_SETVAL(base + GPIO_INPUT_TYPE, pin_bit);
        /* enable interrupt enable bit */
        AIC_REG_SETVAL(base + GPIO_INT_EN, pin_bit);
        /* set pin as input mode */
        AIC_REG_SETVAL(base + GPIO_DIRECTION, pin_bit);
        break;
    case GPIO_TRIGGER_LVL_HIGH:
        /* disable output enable in input mode */
        AIC_REG_CLRVAL(base + GPIO_OUTPUT_EN, pin_bit);
        /* set trigger type as high level mode */
        AIC_REG_SETVAL(base + GPIO_INPUT_VAL, pin_bit);
        /* set trigger type as level triggering mode */
        AIC_REG_SETVAL(base + GPIO_INPUT_TYPE, pin_bit);
        /* enable interrupt enable bit */
        AIC_REG_SETVAL(base + GPIO_INT_EN, pin_bit);
        /* set pin as input mode */
        AIC_REG_SETVAL(base + GPIO_DIRECTION, pin_bit);
        break;
    case GPIO_TRIGGER_EDGE_RISING:
        /* disable output enable in input mode */
        AIC_REG_CLRVAL(base + GPIO_OUTPUT_EN, pin_bit);
        /* set trigger type as rising edge mode */
        AIC_REG_CLRVAL(base + GPIO_INPUT_ON_ANY, pin_bit);
        AIC_REG_SETVAL(base + GPIO_INPUT_VAL, pin_bit);
        /* set trigger type as edge triggering mode */
        AIC_REG_CLRVAL(base + GPIO_INPUT_TYPE, pin_bit);
        /* enable interrupt enable bit */
        AIC_REG_SETVAL(base + GPIO_INT_EN, pin_bit);
        /* set pin as input mode */
        AIC_REG_SETVAL(base + GPIO_DIRECTION, pin_bit);
        break;
    case GPIO_TRIGGER_EDGE_FALLING:
        /* disable output enable in input mode */
        AIC_REG_CLRVAL(base + GPIO_OUTPUT_EN, pin_bit);
        /* set trigger type as falling edge mode */
        AIC_REG_CLRVAL(base + GPIO_INPUT_ON_ANY, pin_bit);
        AIC_REG_CLRVAL(base + GPIO_INPUT_VAL, pin_bit);
        /* set trigger type as edge triggering mode */
        AIC_REG_CLRVAL(base + GPIO_INPUT_TYPE, pin_bit);
        /* enable interrupt enable bit */
        AIC_REG_SETVAL(base + GPIO_INT_EN, pin_bit);
        /* set pin as input mode */
        AIC_REG_SETVAL(base + GPIO_DIRECTION, pin_bit);
        break;
    case GPIO_TRIGGER_EDGE_ANY:
        /* disable output enable in input mode */
        AIC_REG_CLRVAL(base + GPIO_OUTPUT_EN, pin_bit);
        /* set trigger type as any edge mode */
        AIC_REG_CLRVAL(base + GPIO_INPUT_VAL, pin_bit);
        AIC_REG_SETVAL(base + GPIO_INPUT_ON_ANY, pin_bit);
        /* set trigger type as edge triggering mode */
        AIC_REG_CLRVAL(base + GPIO_INPUT_TYPE, pin_bit);
        /* enable interrupt enable bit */
        AIC_REG_SETVAL(base + GPIO_INT_EN, pin_bit);
        /* set pin as input mode */
        AIC_REG_SETVAL(base + GPIO_DIRECTION, pin_bit);
        break;
    case GPIO_BYPASS:
        AIC_REG_SETVAL(base + GPIO_BYPASS_EN, pin_bit);
        break;
    case GPIO_INPUT_NOINT:
        /* disable output enable in input mode */
        AIC_REG_CLRVAL(base + GPIO_OUTPUT_EN, pin_bit);
        /* disable interrupt enable bit */
        AIC_REG_CLRVAL(base + GPIO_INT_EN, pin_bit);
        /* set pin as input mode */
        AIC_REG_SETVAL(base + GPIO_DIRECTION, pin_bit);
        break;
    default:
        ASSERT(0);
        break;
    }
}

u32 aic_gpio_phy_get_value(GPIO_PIN_ID index)
{
    GPIO_IP ip = _gpio_ip_get(index);
    u32 reg_val, base, pin_bit;

    pin_bit = _gpio_index_to_pin(index);
    base    = gpio_bd_cfg[ip].base;

    reg_val = AIC_READ32(base + GPIO_INTPUT_VAL);
    if (reg_val & pin_bit)
        return 1;
    else
        return 0;
}

u32 aic_gpio_phy_get_int_state(GPIO_IP ip)
{
    u32 base;

    base = gpio_bd_cfg[ip].base;

    return AIC_READ32(base + GPIO_INT_MASK);
}

void aic_gpio_phy_int_disable(GPIO_PIN_ID index)
{
    GPIO_IP ip = _gpio_ip_get(index);
    u32 base, pin_bit;

    pin_bit = _gpio_index_to_pin(index);
    base    = gpio_bd_cfg[ip].base;

    AIC_REG_CLRVAL(base + GPIO_INT_EN, pin_bit);
}

