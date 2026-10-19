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
#define AIC_THIS_FILE    0x07bc24
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

#define BIT_PAD_CH      BIT_13 | BIT_14
#define BIT_PAD_CL      BIT_14

extern struct gpio_register_config g_gpio_config[GPIO_PIN_MAX];

static GPIO_IP _gpio_ip_get(GPIO_PIN_ID index)
{
    u32 i, pin_cnt = 0;

    for (i = 0; i < GPIO_IP_MAX; i++) {
        pin_cnt += gpio_bd_cfg[i].pin_cnt;
        if (index < pin_cnt) {
            return gpio_bd_cfg[i].index;
        }
    }

    AIC_ASSERT(0);
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

    AIC_ASSERT(0);
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

    AIC_ASSERT(0);
    return 0;
}

static void aic_gpio_phy_pin_cfg(GPIO_PIN_ID index, GPIO_MODE mode)
{
#ifdef CONFIG_SUBSYS_AP
    struct aic_gpio_aon_pin_map *aon_pin = NULL;
    struct aic_gpio_pd_pin_map *pd_pin = NULL;
    u32 func, val;

    if (index < GPIO_AON_CNT) {
        aon_pin = &gpio_aon_map[index];
        func = AIC_READ32(aon_pin->func_base);
        if (func == aon_pin->func_val) {
            val = AIC_READ32(aon_pin->ctrl_base);
            val &= ~GPIO_PIN_CLR_MASK;
            if (mode == GPIO_OUTPUT_LVL_LOW)
                val |= BIT_PAD_CL | BIT_PAD_SLP_TOP;
            else
                val |= BIT_PAD_CH | BIT_PAD_SLP_TOP;
            AIC_WRITE32(aon_pin->ctrl_base, val);
        } else {
            if (!aon_pin->multi_func_base){
                aicx_info(0x1a3f3f, "d", "GPIO%d pin function is not GPIO, pin config failed!!!\n");
                return;
            }
            func = AIC_READ32(aon_pin->multi_func_base);
            if (func == aon_pin->multi_func_val) {
                val = AIC_READ32(aon_pin->multi_ctrl_base);
                val &= ~GPIO_PIN_CLR_MASK;
                if (mode == GPIO_OUTPUT_LVL_LOW)
                    val |= BIT_PAD_CL | BIT_PAD_SLP_TOP;
                else
                    val |= BIT_PAD_CH | BIT_PAD_SLP_TOP;
                AIC_WRITE32(aon_pin->multi_ctrl_base, val);
            } else {
                aicx_info(0x1a3f3f, "d", "GPIO%d pin function is not GPIO, pin config failed!!!\n");
            }
        }
    }else {
        pd_pin = &gpio_pd_map[index - GPIO_AON_CNT];
        func = AIC_READ32(pd_pin->func_base);
        /* all pd gpio function is 3 */
        if (func == 3) {
            val = AIC_READ32(pd_pin->ctrl_base);
            val &= ~GPIO_PIN_CLR_MASK;
            if (mode == GPIO_OUTPUT_LVL_LOW)
                val |= BIT_PAD_CL | BIT_PAD_SLP_TOP;
            else
                val |= BIT_PAD_CH | BIT_PAD_SLP_TOP;
            AIC_WRITE32(pd_pin->ctrl_base, val);
        } else {
            aicx_info(0x1a3f3f, "d", "GPIO%d pin function is not GPIO, pin config failed!!!\n");
        }
    }
#endif
}

void aic_gpio_phy_set_mode_raw(GPIO_PIN_ID index, GPIO_MODE mode)
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
    default:
        break;
    }
}

void aic_gpio_phy_set_mode(GPIO_PIN_ID index, GPIO_MODE mode)
{
    GPIO_IP ip = _gpio_ip_get(index);
    u32 base, pin_bit;

    pin_bit = _gpio_index_to_pin(index);
    base    = gpio_bd_cfg[ip].base;

    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE

    switch (mode) {
    case GPIO_OUTPUT_LVL_LOW:
        /* set pin as output low */
        AIC_REG_CLRVAL(base + GPIO_OVR, pin_bit);
        /* set pin as output enable */
        AIC_REG_SETVAL(base + GPIO_OER, pin_bit);
        /* set pin as output mode */
        AIC_REG_CLRVAL(base + GPIO_DMR, pin_bit);
        /* config gpio pin */
        aic_gpio_phy_pin_cfg(index, GPIO_OUTPUT_LVL_LOW);
        break;
    case GPIO_OUTPUT_LVL_HIGH:
        /* set pin as output high */
        AIC_REG_SETVAL(base + GPIO_OVR, pin_bit);
        /* set pin as output enable */
        AIC_REG_SETVAL(base + GPIO_OER, pin_bit);
        /* set pin as output mode */
        AIC_REG_CLRVAL(base + GPIO_DMR, pin_bit);
        /* config gpio pin */
        aic_gpio_phy_pin_cfg(index, GPIO_OUTPUT_LVL_HIGH);
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
        AIC_ASSERT(0);
        break;
    }

    AIC_INT_RESTORE
}

GPIO_MODE aic_gpio_phy_get_mode(GPIO_PIN_ID index)
{
    GPIO_IP ip = _gpio_ip_get(index);
    GPIO_MODE mode;
    u32 base, pin_bit, reg_val;

    pin_bit = _gpio_index_to_pin(index);
    base    = gpio_bd_cfg[ip].base;

    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE

    reg_val = AIC_READ32(base + GPIO_DMR);
    /* output mode */
    if (!(reg_val & pin_bit)) {
        reg_val = AIC_READ32(base + GPIO_OER);
        if (!(reg_val & pin_bit)) {
            mode = GPIO_OUTPUT_NONE;
        } else {
            reg_val = AIC_READ32(base + GPIO_OVR);
            if (!(reg_val & pin_bit)) {
                mode = GPIO_OUTPUT_LVL_LOW;
            } else {
                mode = GPIO_OUTPUT_LVL_HIGH;
            }
        }
        AIC_INT_RESTORE
        return mode;
    }

    /* input mode */
    reg_val = AIC_READ32(base + GPIO_ITR);
    if (reg_val & pin_bit) {
        /* level trigger mode */
        reg_val = AIC_READ32(base + GPIO_IVR);
        if (reg_val & pin_bit) {
            mode = GPIO_TRIGGER_LVL_HIGH;
        } else {
            mode = GPIO_TRIGGER_LVL_LOW;
        }
    } else {
        /* edge trigger mode */
        reg_val = AIC_READ32(base + GPIO_IOAR);
        if (reg_val & pin_bit) {
            mode = GPIO_TRIGGER_EDGE_ANY;
        } else {
            reg_val = AIC_READ32(base + GPIO_IVR);
            if (reg_val & pin_bit) {
                mode = GPIO_TRIGGER_EDGE_RISING;
            } else {
                mode = GPIO_TRIGGER_EDGE_FALLING;
            }
        }
    }

    AIC_INT_RESTORE
    return mode;
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
    u32 base, mask;

    base = gpio_bd_cfg[ip].base;
    mask = AIC_READ32(base + GPIO_IMR);

    return AIC_READ32(base + GPIO_ISR) & ~mask;
}

void aic_gpio_phy_int_disable(GPIO_PIN_ID index)
{
    GPIO_IP ip = _gpio_ip_get(index);
    u32 base, pin_bit;

    pin_bit = _gpio_index_to_pin(index);
    base    = gpio_bd_cfg[ip].base;

    AIC_WRITE32(base + GPIO_IDR, pin_bit);
}

u32 aic_gpio_phy_get_pulse_def(void)
{
    return GPIO_PULSE_DEF_INDEX;
}

#ifdef CONFIG_SUBSYS_AP
__attribute__ ((section(".idle_data")))
static u32 top_gpio_bkup[7];
void aic_gpio_phy_suspend(void *p)
{
    u32 base = gpio_bd_cfg[GPIO_TOP].base;

    /* backup top gpio register values */
    top_gpio_bkup[0] = AIC_READ32(base + GPIO_DMR);
    top_gpio_bkup[1] = AIC_READ32(base + GPIO_OER);
    top_gpio_bkup[2] = AIC_READ32(base + GPIO_OVR);
    top_gpio_bkup[3] = AIC_READ32(base + GPIO_IMR);
    top_gpio_bkup[4] = AIC_READ32(base + GPIO_ITR);
    top_gpio_bkup[5] = AIC_READ32(base + GPIO_IVR);
    top_gpio_bkup[6] = AIC_READ32(base + GPIO_IOAR);
}

__attribute__ ((section(".idle_text")))
void aic_gpio_phy_resume(void *p)
{
    u32 base = gpio_bd_cfg[GPIO_TOP].base;

    /* enable global eb */
    AIC_WRITE32(gpio_bd_cfg[GPIO_TOP].glb_base, gpio_bd_cfg[GPIO_TOP].glb_bit);
    /* retention top gpio register values */
    /* 1. retention trigger registers first */
    AIC_WRITE32(base + GPIO_IOAR,   top_gpio_bkup[6]);
    AIC_WRITE32(base + GPIO_IVR,    top_gpio_bkup[5]);
    AIC_WRITE32(base + GPIO_ITR,    top_gpio_bkup[4]);
    /* 2. retention other registers */
    AIC_WRITE32(base + GPIO_OVR,    top_gpio_bkup[2]);
    AIC_WRITE32(base + GPIO_OER,    top_gpio_bkup[1]);
    AIC_WRITE32(base + GPIO_IER,    ~top_gpio_bkup[3]);
    AIC_WRITE32(base + GPIO_DMR,    top_gpio_bkup[0]);
    __DSB();
}
#endif

ISR_RET_E aic_gpio_phy_irq_handler(void *param)
{
    struct aic_gpio_board_cfg *cfg = (struct aic_gpio_board_cfg *)param;
    GPIO_CBFUN gpio_cb;
    GPIO_MODE mode;
    void *gpio_p;
    u32 num, val, base, index;

    AIC_ASSERT(cfg);

    base = cfg->base;

    val = aic_gpio_phy_get_int_state(cfg->index);
    for (num = 0; val != 0; val &= ~(1 << num)) {
        num = aic_find_first_bit(val);
        index = aic_gpio_phy_pin_to_index(cfg->index, num);
        AIC_ASSERT(index < GPIO_PIN_MAX);

        gpio_cb = g_gpio_config[index].cb;
        gpio_p = g_gpio_config[index].param;
        AIC_ASSERT(gpio_cb);

        if (gpio_p)
            gpio_cb(gpio_p);
        else
            gpio_cb((void *)index);
    }

    /*
     * why need to read again, because read clear register(int_val) cannot
     * be cleared once when level trigger mode, it needs to be cleared again.
     */
    val = aic_gpio_phy_get_int_state(cfg->index);
    for (num = 0; val != 0; val &= ~(1 << num)) {
        num = aic_find_first_bit(val);
        index = aic_gpio_phy_pin_to_index(cfg->index, num);
        AIC_ASSERT(index < GPIO_PIN_MAX);

        mode = aic_gpio_phy_get_mode(index);
        if (mode == GPIO_TRIGGER_LVL_LOW || mode == GPIO_TRIGGER_LVL_HIGH)
            continue;

        gpio_cb = g_gpio_config[index].cb;
        gpio_p = g_gpio_config[index].param;
        AIC_ASSERT(gpio_cb);

        if (gpio_p)
            gpio_cb(gpio_p);
        else
            gpio_cb((void *)index);
    }

    return NONE_WORKER;
}

void aic_gpio_phy_init(void)
{
    u32 i;

    for (i = 0; i < GPIO_IP_MAX; i++) {
        if (gpio_bd_cfg[i].irq_no)
            register_irq(gpio_bd_cfg[i].irq_no, aic_gpio_phy_irq_handler, &gpio_bd_cfg[i], NULL, NULL);
        /* enable global eb */
        AIC_WRITE32(gpio_bd_cfg[i].glb_base, gpio_bd_cfg[i].glb_bit);
    }

#ifdef CONFIG_SUBSYS_AP
    slp_register_sleep_hook(aic_gpio_phy_suspend, NULL, NULL, NULL);
#endif
}

