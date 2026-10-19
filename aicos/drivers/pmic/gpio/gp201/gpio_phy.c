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
#define AIC_THIS_FILE    0x018517
#define LOG_MODULE    AMOD_PMIC

#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_cmdline.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <core_op.h>
#include <pmic_io.h>
#include <pmic_irq.h>
#include <slp_inc.h>

#include <gpio_inc.h>
#include <gpio_pmic_cfg.h>

/* GPIO registers define */
#define GPIO_BMR            0x0
#define GPIO_DMR            0x4
#define GPIO_OER            0x8
#define GPIO_OVR            0xC
#define GPIO_INPVR          0x10
#define GPIO_INT_EN         0x14
#define GPIO_INT_RAW        0x18
#define GPIO_INT_MASK       0x1C
#define GPIO_INT_CLR        0x20
#define GPIO_ITR            0x24
#define GPIO_IVR            0x28
#define GPIO_IOAR           0x2C

extern struct gpio_register_config g_gpio_pmic_config[GPIO_PMIC_PIN_MAX];
aos_semaphore_ptr g_gpio_sem;

static inline void _gpio_pmic_set(u32 addr, u32 val)
{
    u32 value;

    value = pmic_readl(addr);
    value |= val;
    pmic_writel( addr, value);
}

static inline void _gpio_pmic_clr(u32 addr, u32 val)
{
    u32 value;

    value = pmic_readl(addr);
    value &= ~val;
    pmic_writel( addr, value);
}

static GPIO_PMIC_IP _gpio_pmic_ip_get(GPIO_PMIC_PIN_ID index)
{
    u32 i, pin_cnt = 0;

    for (i = 0; i < GPIO_IP_MAX; i++) {
        pin_cnt += gpio_pmic_bd_cfg[i].pin_cnt;
        if (index < pin_cnt) {
            return gpio_pmic_bd_cfg[i].index;
        }
    }

    return 0;
}

static u32 _gpio_pmic_index_to_pin(GPIO_PMIC_PIN_ID index)
{
    u32 i, pin_cnt;

    pin_cnt = 0;
    for (i = 0; i < GPIO_IP_MAX; i++) {
        pin_cnt += gpio_pmic_bd_cfg[i].pin_cnt;
        /* why -1? becasue pin index is start from 0 */
        if (index > pin_cnt - 1) {
            continue;
        } else {
            return BIT(index + gpio_pmic_bd_cfg[i].pin_cnt - pin_cnt);
        }
    }

    return 0;
}

u32 aic_gpio_pmic_phy_pin_to_index(GPIO_PMIC_PIN_ID ip_id, u32 pin_offset)
{
    u32 i, pin_index = pin_offset;

    for (i = 0; i < GPIO_IP_MAX; i++) {
        if (ip_id > gpio_pmic_bd_cfg[i].index) {
            pin_index += gpio_pmic_bd_cfg[i].pin_cnt;
        } else {
            return pin_index;
        }
    }

    return 0;
}

void aic_gpio_pmic_phy_set_mode(GPIO_PMIC_PIN_ID index, GPIO_MODE mode)
{
    GPIO_PMIC_IP ip = _gpio_pmic_ip_get(index);
    u32 base, pin_bit;

    pin_bit = _gpio_pmic_index_to_pin(index);
    base    = gpio_pmic_bd_cfg[ip].base;

    if (ip == GPIO_PMIC_PD)
        pmic_writel(gpio_pmic_bd_cfg[ip].glb_base, gpio_pmic_bd_cfg[ip].glb_bit);

    switch (mode) {
    case GPIO_OUTPUT_LVL_LOW:
        /* set pin as output mode */
        _gpio_pmic_clr(base + GPIO_DMR, pin_bit);
        /* set pin as output low */
        _gpio_pmic_clr(base + GPIO_OVR, pin_bit);
        /* set pin as output enable */
        _gpio_pmic_set(base + GPIO_OER, pin_bit);
        break;
    case GPIO_OUTPUT_LVL_HIGH:
        /* set pin as output mode */
        _gpio_pmic_clr(base + GPIO_DMR, pin_bit);
        /* set pin as output high */
        _gpio_pmic_set(base + GPIO_OVR, pin_bit);
        /* set pin as output enable */
        _gpio_pmic_set(base + GPIO_OER, pin_bit);
        break;
    case GPIO_TRIGGER_LVL_LOW:
        /* disable output enable in input mode */
        _gpio_pmic_clr(base + GPIO_OER, pin_bit);
        /* set trigger type as low level mode */
        _gpio_pmic_clr(base + GPIO_IVR, pin_bit);
        /* set trigger type as level triggering mode */
        _gpio_pmic_set(base + GPIO_ITR, pin_bit);
        /* eanble interrupt enable bit */
        _gpio_pmic_set(base + GPIO_INT_EN, pin_bit);
        /* set pin as input mode */
        _gpio_pmic_set(base + GPIO_DMR, pin_bit);
        break;
    case GPIO_TRIGGER_LVL_HIGH:
        /* disable output enable in input mode */
        _gpio_pmic_clr(base + GPIO_OER, pin_bit);
        /* set trigger type as high level mode */
        _gpio_pmic_set(base + GPIO_IVR, pin_bit);
        /* set trigger type as level triggering mode */
        _gpio_pmic_set(base + GPIO_ITR, pin_bit);
        /* eanble interrupt enable bit */
        _gpio_pmic_set(base + GPIO_INT_EN, pin_bit);
        /* set pin as input mode */
        _gpio_pmic_set(base + GPIO_DMR, pin_bit);
        break;
    case GPIO_TRIGGER_EDGE_RISING:
        /* disable output enable in input mode */
        _gpio_pmic_clr(base + GPIO_OER, pin_bit);
        /* set trigger type as rising edge mode */
        _gpio_pmic_clr(base + GPIO_IOAR, pin_bit);
        _gpio_pmic_set(base + GPIO_IVR, pin_bit);
        /* set trigger type as edge triggering mode */
        _gpio_pmic_clr(base + GPIO_ITR, pin_bit);
        /* eanble interrupt enable bit */
        _gpio_pmic_set(base + GPIO_INT_EN, pin_bit);
        /* set pin as input mode */
        _gpio_pmic_set(base + GPIO_DMR, pin_bit);
        break;
    case GPIO_TRIGGER_EDGE_FALLING:
        /* disable output enable in input mode */
        _gpio_pmic_clr(base + GPIO_OER, pin_bit);
        /* set trigger type as falling edge mode */
        _gpio_pmic_clr(base + GPIO_IOAR, pin_bit);
        _gpio_pmic_clr(base + GPIO_IVR, pin_bit);
        /* set trigger type as edge triggering mode */
        _gpio_pmic_clr(base + GPIO_ITR, pin_bit);
        /* eanble interrupt enable bit */
        _gpio_pmic_set(base + GPIO_INT_EN, pin_bit);
        /* set pin as input mode */
        _gpio_pmic_set(base + GPIO_DMR, pin_bit);
        break;
    case GPIO_TRIGGER_EDGE_ANY:
        /* disable output enable in input mode */
        _gpio_pmic_clr(base + GPIO_OER, pin_bit);
        /* set trigger type as any edge mode */
        _gpio_pmic_clr(base + GPIO_IVR, pin_bit);
        _gpio_pmic_set(base + GPIO_IOAR, pin_bit);
        /* set trigger type as edge triggering mode */
        _gpio_pmic_clr(base + GPIO_ITR, pin_bit);
        /* eanble interrupt enable bit */
        _gpio_pmic_set(base + GPIO_INT_EN, pin_bit);
        /* set pin as input mode */
        _gpio_pmic_set(base + GPIO_DMR, pin_bit);
        break;
    case GPIO_BYPASS:
        _gpio_pmic_set(base + GPIO_BMR, pin_bit);
        break;
    case GPIO_INPUT_NOINT:
        /* disable output enable in input mode */
        _gpio_pmic_clr(base + GPIO_OER, pin_bit);
        /* disable interrupt enable bit */
        _gpio_pmic_clr(base + GPIO_INT_EN, pin_bit);
        /* set pin as input mode */
        _gpio_pmic_set(base + GPIO_DMR, pin_bit);
        break;
    default:
        AIC_ASSERT(0);
        break;
    }
}

u32 aic_gpio_pmic_phy_get_value(GPIO_PMIC_PIN_ID index)
{
    GPIO_PMIC_IP ip = _gpio_pmic_ip_get(index);
    u32 reg_val, base, pin_bit;

    pin_bit = _gpio_pmic_index_to_pin(index);
    base    = gpio_pmic_bd_cfg[ip].base;

    reg_val = pmic_readl(base + GPIO_INPVR);
    if (reg_val & pin_bit)
        return 1;
    else
        return 0;
}

u32 aic_gpio_pmic_phy_get_int_state(GPIO_PMIC_IP ip)
{
    u32 base;

    base = gpio_pmic_bd_cfg[ip].base;

    return pmic_readl(base + GPIO_INT_MASK);
}

u32 aic_gpio_pmic_phy_get_pin_int_status(GPIO_PMIC_PIN_ID index)
{
    GPIO_PMIC_IP ip = _gpio_pmic_ip_get(index);
    u32 reg_val, base, pin_bit;

    pin_bit = _gpio_pmic_index_to_pin(index);
    base    = gpio_pmic_bd_cfg[ip].base;
    reg_val = pmic_readl(base + GPIO_INT_MASK);
    _gpio_pmic_set(base + GPIO_INT_CLR, pin_bit);
    _gpio_pmic_clr(base + GPIO_INT_CLR, pin_bit);
    if (reg_val & pin_bit)
        return 1;
    else
        return 0;
}

void aic_gpio_pmic_phy_int_clr(GPIO_PMIC_PIN_ID index)
{
    GPIO_PMIC_IP ip = _gpio_pmic_ip_get(index);
    u32 base, pin_bit;

    pin_bit = _gpio_pmic_index_to_pin(index);
    base    = gpio_pmic_bd_cfg[ip].base;

    _gpio_pmic_set(base + GPIO_INT_CLR, pin_bit);
    _gpio_pmic_clr(base + GPIO_INT_CLR, pin_bit);
}

void aic_gpio_pmic_phy_int_disable(GPIO_PMIC_PIN_ID index)
{
    GPIO_PMIC_IP ip = _gpio_pmic_ip_get(index);
    u32 base, pin_bit;

    pin_bit = _gpio_pmic_index_to_pin(index);
    base    = gpio_pmic_bd_cfg[ip].base;

    _gpio_pmic_clr(base + GPIO_INT_EN, pin_bit);
}

void aic_gpio_pmic_phy_irq_handler(void *param)
{
    struct aic_gpio_pmic_board_cfg *cfg = (struct aic_gpio_pmic_board_cfg *)param;
    GPIO_CBFUN gpio_cb;
    void *gpio_p;
    u32 num, val, base, index;

    AIC_ASSERT(cfg);

    base = cfg->base;

    val = aic_gpio_pmic_phy_get_int_state(cfg->index);
    AIC_ASSERT(val);

    for (num = 0; val != 0; val &= ~(1 << num)) {
        num = aic_find_first_bit(val);

        index = aic_gpio_pmic_phy_pin_to_index(cfg->index, num);
        AIC_ASSERT(index < GPIO_PMIC_PIN_MAX);

        aic_gpio_pmic_phy_int_clr(index);

        gpio_cb = g_gpio_pmic_config[index].cb;
        gpio_p = g_gpio_pmic_config[index].param;

        AIC_ASSERT(gpio_cb);

        if (gpio_p)
            gpio_cb(gpio_p);
        else
            gpio_cb((void *)index);
    }
}

void aic_gpio_pmic_phy_init(void)
{
    u32 i, pmic_id;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    if (((pmic_id >> 16) & 0xFFFF) == 0x202) {
        gpio_pmic_bd_cfg[GPIO_PMIC_AON].base = PMIC_BASE_GP202_GPIO_RTC;
        gpio_pmic_bd_cfg[GPIO_PMIC_AON].glb_base = REG_GLB_UPS_CKM_EN_GLB_CLK_UPS_GPIO_RF;
        gpio_pmic_bd_cfg[GPIO_PMIC_AON].irq_no = PMIC_IRQ_GP202_GPIO_UPS;
        gpio_pmic_bd_cfg[GPIO_PMIC_PD].base = PMIC_BASE_GP202_GPIO_PD;
        gpio_pmic_bd_cfg[GPIO_PMIC_PD].glb_base = REG_GLB_PD_CKM_EN_GLB_CLK_GP202_PD_GPIO_RF;
    }

    for (i = 0; i < GPIO_PMIC_MAX; i++) {
        if (gpio_pmic_bd_cfg[i].irq_no)
            register_pmic_irq(gpio_pmic_bd_cfg[i].irq_no, aic_gpio_pmic_phy_irq_handler, &gpio_pmic_bd_cfg[i]);

        /* set gpio glb */
        pmic_writel(gpio_pmic_bd_cfg[i].glb_base, gpio_pmic_bd_cfg[i].glb_bit);
    }
}

