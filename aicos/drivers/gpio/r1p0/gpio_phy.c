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

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x070ac1
#define LOG_MODULE    AMOD_GPIO

#include <aic_auto_reg.h>
#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <core_op.h>
#include <irq.h>
#include <irq_chip.h>
#include <slp_inc.h>

#include <gpo_inc.h>
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

extern struct gpio_register_config g_gpio_config[GPIO_PIN_MAX];

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

    AIC_ASSERT(0);
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

    AIC_ASSERT(0);
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

    AIC_ASSERT(0);
    return 0;
}

void aic_gpio_phy_set_mode_raw(GPIO_PIN_ID index, GPIO_MODE mode)
{
    GPIO_IP ip;
    u32 base, pin_bit;

    /* if index more than 100, call gpo api */
    if (index >= AIC_GPIO_GPO_MAPPING_ID) {
        aic_gpo_output(index - AIC_GPIO_GPO_MAPPING_ID, mode);
        return;
    }

    ip = _gpio_ip_get(index);
    pin_bit = _gpio_index_to_pin(index);
    base    = gpio_bd_cfg[ip].base;

    switch (mode) {
    case GPIO_OUTPUT_LVL_LOW:
        /* set pin as output mode */
        AIC_REG_CLRVAL(base + GPIO_DIRECTION, pin_bit);
        /* set pin as output low */
        AIC_REG_CLRVAL(base + GPIO_OUTPUT_VAL, pin_bit);
        /* set pin as output enable */
        AIC_REG_SETVAL(base + GPIO_OUTPUT_EN, pin_bit);
        break;
    case GPIO_OUTPUT_LVL_HIGH:
        /* set pin as output mode */
        AIC_REG_CLRVAL(base + GPIO_DIRECTION, pin_bit);
        /* set pin as output high */
        AIC_REG_SETVAL(base + GPIO_OUTPUT_VAL, pin_bit);
        /* set pin as output enable */
        AIC_REG_SETVAL(base + GPIO_OUTPUT_EN, pin_bit);
        break;
    default:
        break;
    }
}

void aic_gpio_phy_set_mode(GPIO_PIN_ID index, GPIO_MODE mode)
{
    GPIO_IP ip;
    u32 base, pin_bit;

    /* if index more than 100, call gpo api */
    if (index >= AIC_GPIO_GPO_MAPPING_ID) {
        aic_gpo_output(index - AIC_GPIO_GPO_MAPPING_ID, mode);
        return;
    }

    ip = _gpio_ip_get(index);
    pin_bit = _gpio_index_to_pin(index);
    base    = gpio_bd_cfg[ip].base;

    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE

    switch (mode) {
    case GPIO_OUTPUT_LVL_LOW:
        /* set pin as output mode */
        AIC_REG_CLRVAL(base + GPIO_DIRECTION, pin_bit);
        /* set pin as output low */
        AIC_REG_CLRVAL(base + GPIO_OUTPUT_VAL, pin_bit);
        /* set pin as output enable */
        AIC_REG_SETVAL(base + GPIO_OUTPUT_EN, pin_bit);
        break;
    case GPIO_OUTPUT_LVL_HIGH:
        /* set pin as output mode */
        AIC_REG_CLRVAL(base + GPIO_DIRECTION, pin_bit);
        /* set pin as output high */
        AIC_REG_SETVAL(base + GPIO_OUTPUT_VAL, pin_bit);
        /* set pin as output enable */
        AIC_REG_SETVAL(base + GPIO_OUTPUT_EN, pin_bit);
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
        AIC_ASSERT(0);
        break;
    }

    AIC_INT_RESTORE
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

u32 aic_gpio_phy_get_pin_int_status(GPIO_PIN_ID index)
{
    GPIO_IP ip = _gpio_ip_get(index);
    u32 reg_val, base, pin_bit;

    pin_bit = _gpio_index_to_pin(index);
    base    = gpio_bd_cfg[ip].base;
    reg_val = AIC_READ32(base + GPIO_INT_MASK);
    AIC_REG_SETVAL(base + GPIO_INT_CLR, pin_bit);
    AIC_REG_CLRVAL(base + GPIO_INT_CLR, pin_bit);
    if (reg_val & pin_bit)
        return 1;
    else
        return 0;
}

void aic_gpio_phy_int_clr(GPIO_PIN_ID index)
{
    GPIO_IP ip = _gpio_ip_get(index);
    u32 base, pin_bit;

    pin_bit = _gpio_index_to_pin(index);
    base    = gpio_bd_cfg[ip].base;

    AIC_REG_SETVAL(base + GPIO_INT_CLR, pin_bit);
    AIC_REG_CLRVAL(base + GPIO_INT_CLR, pin_bit);
}

void aic_gpio_phy_int_disable(GPIO_PIN_ID index)
{
    GPIO_IP ip = _gpio_ip_get(index);
    u32 base, pin_bit;

    pin_bit = _gpio_index_to_pin(index);
    base    = gpio_bd_cfg[ip].base;

    AIC_REG_CLRVAL(base + GPIO_INT_EN, pin_bit);
}

u32 aic_gpio_phy_get_pulse_def(void)
{
    return GPIO_PULSE_DEF_INDEX;
}

#ifdef CONFIG_SUBSYS_AP
static u32 top_gpio_bkup[13];
void aic_gpio_phy_suspend(void *p)
{
    GPIO_IP ip = _gpio_ip_get(GPIO_TOP);
    u32 i, base = gpio_bd_cfg[ip].base;

    /* backup top gpio register values */
    for (i = 0; i < 13; i++) {
        top_gpio_bkup[i] = AIC_READ32(base + i * 4);
    }
}

void aic_gpio_phy_resume(void *p)
{
    GPIO_IP ip = _gpio_ip_get(GPIO_TOP);
    u32 base = gpio_bd_cfg[ip].base;

    /* enable global eb */
    AIC_WRITE32(gpio_bd_cfg[ip].glb_base, gpio_bd_cfg[ip].glb_bit);
    /* retention top gpio register values */
    /* 1. retention trigger registers first */
    AIC_WRITE32(base + GPIO_INPUT_ON_ANY,   *(volatile u32 *)(top_gpio_bkup + (GPIO_INPUT_ON_ANY / 4)));
    AIC_WRITE32(base + GPIO_INPUT_VAL,      *(volatile u32 *)(top_gpio_bkup + (GPIO_INPUT_VAL / 4)));
    AIC_WRITE32(base + GPIO_INPUT_TYPE,     *(volatile u32 *)(top_gpio_bkup + (GPIO_INPUT_TYPE / 4)));
    /* 2. retention other registers */
    AIC_WRITE32(base + GPIO_OUTPUT_VAL,     *(volatile u32 *)(top_gpio_bkup + (GPIO_OUTPUT_VAL / 4)));
    AIC_WRITE32(base + GPIO_OUTPUT_EN,      *(volatile u32 *)(top_gpio_bkup + (GPIO_OUTPUT_EN / 4)));
    AIC_WRITE32(base + GPIO_INT_EN,         *(volatile u32 *)(top_gpio_bkup + (GPIO_INT_EN / 4)));
    AIC_WRITE32(base + GPIO_DIRECTION,      *(volatile u32 *)(top_gpio_bkup + (GPIO_DIRECTION / 4)));
}
#endif

ISR_RET_E aic_gpio_phy_irq_handler(void *param)
{
    struct aic_gpio_board_cfg *cfg = (struct aic_gpio_board_cfg *)param;
    GPIO_CBFUN gpio_cb;
    void *gpio_p;
    u32 num, val, base;

    AIC_ASSERT(cfg);

    base = cfg->base;

    val = AIC_READ32(base + GPIO_INT_STATUS);
    AIC_ASSERT(val);

    for (num = 0; val != 0; val &= ~(1 << num)) {
        num = aic_find_first_bit(val);

        AIC_REG_SETVAL(base + GPIO_INT_CLR, BIT(num));
        AIC_REG_CLRVAL(base + GPIO_INT_CLR, BIT(num));

        gpio_cb = g_gpio_config[num].cb;
        gpio_p = g_gpio_config[num].param;

        AIC_ASSERT(gpio_cb);

        if (gpio_p)
            gpio_cb(gpio_p);
        else
            gpio_cb((void *)num);
    }

    return NONE_WORKER;
}

void aic_gpio_phy_init(void)
{
    u32 i, ip_max = sizeof(gpio_bd_cfg) / sizeof(*gpio_bd_cfg);

    for (i = 0; i < ip_max; i++) {
        if (gpio_bd_cfg[i].irq_no)
            register_irq(gpio_bd_cfg[i].irq_no, aic_gpio_phy_irq_handler, &gpio_bd_cfg[i], NULL, NULL);
        /* enable global eb */
        AIC_WRITE32(gpio_bd_cfg[i].glb_base, gpio_bd_cfg[i].glb_bit);
    }

#ifdef CONFIG_SUBSYS_AP
    slp_register_sleep_hook(aic_gpio_phy_suspend, NULL, aic_gpio_phy_resume, NULL);
#endif
}

