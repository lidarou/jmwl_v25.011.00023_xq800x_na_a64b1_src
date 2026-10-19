/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
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
#define GPIO_BMR            0x0
#define GPIO_DMR            0x4
#define GPIO_OER            0x8
#define GPIO_OVR            0xC
#define GPIO_INPVR          0x10
#define GPIO_IMR            0x14
#define GPIO_IER            0x18
#define GPIO_IDR            0x1C
#define GPIO_ISR            0x20
#define GPIO_ITR            0x24
#define GPIO_IVR            0x28
#define GPIO_IOAR           0x2C

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

static u32 _gpio_index_to_pin(GPIO_PIN_ID index)
{
    u32 i, pin_cnt;

    pin_cnt = 0;
    for (i = 0; i < GPIO_IP_MAX; i++) {
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
    u32 i, pin_index = pin_offset;

    for (i = 0; i < GPIO_IP_MAX; i++) {
        if (ip_id > gpio_bd_cfg[i].index) {
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
        /* set pin as output low */
        AIC_REG_CLRVAL(base + GPIO_OVR, pin_bit);
        /* set pin as output enable */
        AIC_REG_SETVAL(base + GPIO_OER, pin_bit);
        /* set pin as output mode */
        AIC_REG_CLRVAL(base + GPIO_DMR, pin_bit);
        break;
    case GPIO_OUTPUT_LVL_HIGH:
        /* set pin as output high */
        AIC_REG_SETVAL(base + GPIO_OVR, pin_bit);
        /* set pin as output enable */
        AIC_REG_SETVAL(base + GPIO_OER, pin_bit);
        /* set pin as output mode */
        AIC_REG_CLRVAL(base + GPIO_DMR, pin_bit);
        break;
    case GPIO_TRIGGER_LVL_LOW:
        /* disable output enable in input mode */
        AIC_REG_CLRVAL(base + GPIO_OER, pin_bit);
        /* set trigger type as low level mode */
        AIC_REG_CLRVAL(base + GPIO_IVR, pin_bit);
        /* set trigger type as level triggering mode */
        AIC_REG_SETVAL(base + GPIO_ITR, pin_bit);
        /* enable interrupt enable bit */
        AIC_WRITE32(base + GPIO_IER, pin_bit);
        /* set pin as input mode */
        AIC_REG_SETVAL(base + GPIO_DMR, pin_bit);
        break;
    case GPIO_TRIGGER_LVL_HIGH:
        /* disable output enable in input mode */
        AIC_REG_CLRVAL(base + GPIO_OER, pin_bit);
        /* set trigger type as high level mode */
        AIC_REG_SETVAL(base + GPIO_IVR, pin_bit);
        /* set trigger type as level triggering mode */
        AIC_REG_SETVAL(base + GPIO_ITR, pin_bit);
        /* enable interrupt enable bit */
        AIC_WRITE32(base + GPIO_IER, pin_bit);
        /* set pin as input mode */
        AIC_REG_SETVAL(base + GPIO_DMR, pin_bit);
        break;
    case GPIO_TRIGGER_EDGE_RISING:
        /* disable output enable in input mode */
        AIC_REG_CLRVAL(base + GPIO_OER, pin_bit);
        /* set trigger type as rising edge mode */
        AIC_REG_CLRVAL(base + GPIO_IOAR, pin_bit);
        AIC_REG_SETVAL(base + GPIO_IVR, pin_bit);
        /* set trigger type as edge triggering mode */
        AIC_REG_CLRVAL(base + GPIO_ITR, pin_bit);
        /* enable interrupt enable bit */
        AIC_WRITE32(base + GPIO_IER, pin_bit);
        /* set pin as input mode */
        AIC_REG_SETVAL(base + GPIO_DMR, pin_bit);
        break;
    case GPIO_TRIGGER_EDGE_FALLING:
        /* disable output enable in input mode */
        AIC_REG_CLRVAL(base + GPIO_OER, pin_bit);
        /* set trigger type as falling edge mode */
        AIC_REG_CLRVAL(base + GPIO_IOAR, pin_bit);
        AIC_REG_CLRVAL(base + GPIO_IVR, pin_bit);
        /* set trigger type as edge triggering mode */
        AIC_REG_CLRVAL(base + GPIO_ITR, pin_bit);
        /* enable interrupt enable bit */
        AIC_WRITE32(base + GPIO_IER, pin_bit);
        /* set pin as input mode */
        AIC_REG_SETVAL(base + GPIO_DMR, pin_bit);
        break;
    case GPIO_TRIGGER_EDGE_ANY:
        /* disable output enable in input mode */
        AIC_REG_CLRVAL(base + GPIO_OER, pin_bit);
        /* set trigger type as any edge mode */
        AIC_REG_CLRVAL(base + GPIO_IVR, pin_bit);
        AIC_REG_SETVAL(base + GPIO_IOAR, pin_bit);
        /* set trigger type as edge triggering mode */
        AIC_REG_CLRVAL(base + GPIO_ITR, pin_bit);
        /* enable interrupt enable bit */
        AIC_WRITE32(base + GPIO_IER, pin_bit);
        /* set pin as input mode */
        AIC_REG_SETVAL(base + GPIO_DMR, pin_bit);
        break;
    case GPIO_BYPASS:
        AIC_REG_SETVAL(base + GPIO_BMR, pin_bit);
        break;
    case GPIO_INPUT_NOINT:
        /* disable output enable in input mode */
        AIC_REG_CLRVAL(base + GPIO_OER, pin_bit);
        /* disable interrupt enable bit */
        AIC_WRITE32(base + GPIO_IDR, pin_bit);
        /* set pin as input mode */
        AIC_REG_SETVAL(base + GPIO_DMR, pin_bit);
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

    reg_val = AIC_READ32(base + GPIO_INPVR);
    if (reg_val & pin_bit)
        return 1;
    else
        return 0;
}

u32 aic_gpio_phy_get_int_state(GPIO_IP ip)
{
    u32 base;

    base = gpio_bd_cfg[ip].base;

    return AIC_READ32(base + GPIO_ISR);
}

void aic_gpio_phy_int_disable(GPIO_PIN_ID index)
{
    GPIO_IP ip = _gpio_ip_get(index);
    u32 base, pin_bit;

    pin_bit = _gpio_index_to_pin(index);
    base    = gpio_bd_cfg[ip].base;

    AIC_WRITE32(base + GPIO_IDR, pin_bit);
}

