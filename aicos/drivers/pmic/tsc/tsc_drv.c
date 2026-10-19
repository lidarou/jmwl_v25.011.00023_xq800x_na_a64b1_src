/*****************************************************************************/
/*                                                                           */
/*         Copyright 2022 by AIC Corporation. All rights reserved.           */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the tsc software driver.
*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x08204f
#define LOG_MODULE    AMOD_TSC

#include <aic_auto_reg.h>
#include <aic_bitmap.h>
#include <aic_io.h>
#include <aic_init.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <aic_osa.h>
#include <aos_utl.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include <pmic_irq.h>
#include <tsc.h>
#include <timer_inc.h>
#include "tsc_reg.h"
#include "tsc_cfg.h"
#include <gpio_inc.h>
#include <pin_ctrl.h>
#include <display.h>
#include <pmu_user.h>

#define TSC_DEBUG 0


#define TSC_TASK_PRIORITY               (5)
#define TSC_STACK_SIZE                  (1024)
#define TSC_RELEASE_CHECK_INTERNAL      (30)
#define TSC_GET_Y(x)                    (((x) >> 8) & 0xff)
#define TSC_GET_X(x)                    (((x) & 0xff))

u32 g_tsc_irq_status = 0;
u32 g_tsc_pre_time = 0;
u32 g_tsc_tick_count = 0;
u32 g_tsc_check_tick_count = 0;
bool g_tsc_is_checking_release = false;

aic_tsc_point_t g_tsc_point = {0};
aos_thread_id   g_tsc_thread = AOS_INVALID_THREAD_ID;
aos_semaphore_ptr g_p_tsc_sema = AOS_NULL;
struct pmu_user *tsc_adc_handle = NULL;

/**
* aic_tsc_read_point - Brief description of function.
* @p_tsc_point: output the tsc point info.
* Return: AIC_SUCCESS.
* Description: This function is used to read tsc point.
*/
int aic_tsc_read_point( aic_tsc_point_t *p_tsc_point )
{
    aos_memcpy(p_tsc_point,&g_tsc_point,sizeof(aic_tsc_point_t));

    return AIC_SUCCESS;
}

static int aic_tsc_debounce_cnt_set(int cnt)
{
    if (cnt > BIT_ADC_CTRL_TSC_DEBOUNCE_CHK_NUM(0x1ff))
        return AIC_ENODEV;

    pmic_writel(REG_ADC_CTRL_TSC_DEBOUNCE, BIT_ADC_CTRL_TSC_DEBOUNCE_CHK_NUM(cnt));

    return 0;
}

static int aic_tsc_press_cnt_set(int cnt)
{
    if (cnt > BIT_ADC_CTRL_TSC_PRESS_CHK_NUM(0x1ff))
        return AIC_ENODEV;

    pmic_writel(REG_ADC_CTRL_TSC_PRESS, BIT_ADC_CTRL_TSC_PRESS_CHK_NUM(cnt));

    return 0;
}

static int aic_tsc_trigger_time_set(int time_ms)
{
    if (time_ms > BIT_ADC_CTRL_TSC_NEXT_TRIG_NUM(0x3f))
        return AIC_ENODEV;

    pmic_writel(REG_ADC_CTRL_TSC_TRIG_TIME, BIT_ADC_CTRL_TSC_NEXT_TRIG_NUM(time_ms));

    return 0;
}

static int aic_tsc_pre_chg_time_set(int time_10us)
{
    if (time_10us > BIT_ADC_CTRL_TSC_PRE_CHARGE_NUM(0x1ff))
        return AIC_ENODEV;

    pmic_writel(REG_ADC_CTRL_TSC_CFG_TIME, BIT_ADC_CTRL_TSC_PRE_CHARGE_NUM(time_10us));

    return 0;
}

static void aic_tsc_sample_enabled(bool enabled)
{
    if (enabled)
        pmic_writel(REG_ADC_CTRL_TSC_SAMP_EN, BIT_ADC_CTRL_TSC_SAMP_EN);
    else
        pmic_writel(REG_ADC_CTRL_TSC_SAMP_EN, 0);
}

static void aic_tsc_int_enabled(bool enabled)
{
    if (enabled)
        pmic_writel(REG_ADC_CTRL_INT_EN, pmic_readl(REG_ADC_CTRL_INT_EN) | BIT_ADC_CTRL_TSC_INTR_EN);
    else
        pmic_writel(REG_ADC_CTRL_INT_EN, pmic_readl(REG_ADC_CTRL_INT_EN) & ~BIT_ADC_CTRL_TSC_INTR_EN);
}

static void aic_tsc_int_status_clear(void)
{
    pmic_writel(REG_ADC_CTRL_INT_CLR, BIT_ADC_CTRL_TSC_INTR_CLR);
}

static void aic_tsc_save_point( void )
{
    u8 pos_x = 0;
    u8 pos_y = 0;
    u32 val;
    int ispressed, sampling;

    val = pmic_readl(REG_ADC_CTRL_TSC_DATA);

    pos_x = TSC_GET_X(val);
    pos_y = TSC_GET_Y(val);
    ispressed = (val & BIT_ADC_CTRL_TSC_PRESS_FLAG) >> 16;
    sampling = (val & BIT_ADC_CTRL_TSC_SAMPLING) >> 17;

    if (0 == sampling) {/* adc sample done */
        memset(&g_tsc_point, 0, sizeof(aic_tsc_point_t));
        g_tsc_point.point_event = (1 == ispressed) ? TSC_POINT_EVENT_PRESS:TSC_POINT_EVENT_RELEASE;
        g_tsc_point.tick_count = aic_get_tick();

        /* TODO: add another rotation type,here hard code for mirror_y */
        g_tsc_point.point_x = pos_x;
        g_tsc_point.point_y = TSC_POS_MAX_VALUE_Y - pos_y;
    }

    g_tsc_tick_count = aic_get_tick();

#if TSC_DEBUG
        aicx_info(0x0f7327, "xddddxd", "REG_ADC_CTRL_TSC_DATA:0x%x x:%d y:%d is_presssd:%d sampling:%d\n adc_dgb:0x%x  tick_count = %d \n", val,
                    pos_x, pos_y, ispressed, sampling, pmic_readl(REG_ADC_CTRL_ADC_DBG),g_tsc_tick_count);
        aicx_info(0x15ac43, "dd", "reverse positon: x:%d y:%d \n ", g_tsc_point.point_x,g_tsc_point.point_y);
#endif

}

void aic_tsc_reset(void)
{
    pmic_writel(REG_GLB_PD_SW_RST_GLB_CLK_APB_ADC, BIT_GLB_PD_SW_RST_GLB_CLK_APB_ADC);
    pmic_writel(REG_GLB_PD_SW_RST_GLB_CLK_APB_ADC, 0);
}

void tsc_thread_entry(void *thread_input)
{
    u32 val;
    int ispressed;
    u32 ret;

    while (1) {

        if (false == g_tsc_is_checking_release) {
            /* wait for point is pressed */
            ret = aos_get_semaphore(g_p_tsc_sema, AOS_WAIT_FOREVER);
            if (ret) {
                aicx_error(0x100b89, "x", "tsc get semaphore fail 0x%x\n", ret);
                continue;
            }
        }

        /* start check release cause point is pressed */
        g_tsc_is_checking_release = true;
        g_tsc_check_tick_count = aic_get_tick();
        if ((TSC_POINT_EVENT_PRESS == g_tsc_point.point_event ) && ABS((s32)(g_tsc_check_tick_count - g_tsc_tick_count)) > TSC_RELEASE_CHECK_INTERNAL) {
            /* check if point is released */
            val = pmic_readl(REG_ADC_CTRL_TSC_DATA);
            ispressed = (val & BIT_ADC_CTRL_TSC_PRESS_FLAG) >> 16;
            if (0 == ispressed) {
                g_tsc_point.point_event = TSC_POINT_EVENT_RELEASE;

                /* stop check cause point is released */
                g_tsc_is_checking_release = false;

#if TSC_DEBUG
                aicx_info(0x111b45, "d", "point is released, tick_count = %d \n", g_tsc_check_tick_count);
#endif
            }
        }
        aos_sleep(TSC_RELEASE_CHECK_INTERNAL);
    }
}


void aic_tsc_irq_handler(void *val)
{
    g_tsc_irq_status = pmic_readl(REG_ADC_CTRL_ADC_INT_STS);
    aic_tsc_int_status_clear();

#if TSC_DEBUG
        aicx_info(0x0d6ba4, "x", "tsc irq_status:0x%x\n", g_tsc_irq_status);
#endif

    /* first press event come, put semaphore to start check release event */
    if (false == g_tsc_is_checking_release) {
        aos_put_semaphore(g_p_tsc_sema);
    }

    aic_tsc_save_point();

    aic_tsc_int_status_clear();
}

int tsc_adc_enable(bool enable)
{
    int ret;

    if (!tsc_adc_handle) {
        tsc_adc_handle = pmu_get("tsc_adc", "adc");
        if (!tsc_adc_handle) {
            ret = AIC_ENXIO;
            aicx_warn(0x0f8060, "s", "%s get adc pmu operate handler fail!\n", __func__);
            goto out;
        }
    }

    if (enable)
        ret = pmu_enable(tsc_adc_handle);
    else
        ret = pmu_disable(tsc_adc_handle);

    if (ret)
        aicx_warn(0x0daf2f, "s", "%s adc pmu operate fail!\n", __func__);

out:
    return ret;
}

void bl_bound_adc_func(AIC_BL_SWITCH bl_status)
{
    int ret;

    switch (bl_status) {
    case AIC_BL_TURN_ON:
        ret = tsc_adc_enable(true);
        if (ret)
            aicx_warn(0x1125d5, "s", "%s adc enable fail!\n", __func__);
        break;

    case AIC_BL_TURN_OFF:
        ret = tsc_adc_enable(false);
        if (ret)
            aicx_warn(0x0a099a, "s", "%s adc disable fail!\n", __func__);
        break;

    default:
        aicx_warn(0x0699dd, "s", "%s unsupport bl status cb!\n", __func__);
        break;
    }

}

int aic_tsc_irq_init(void)
{
    int ret;

    aic_tsc_sample_enabled(false);
    aic_gpio_pmic_request(GPIO_PMIC_PD_6);
    aic_gpio_pmic_output(GPIO_PMIC_PD_6, GPIO_OUTPUT_LVL_HIGH);
    aic_tsc_press_cnt_set(tsc_cfg.press_cnt);
    aic_tsc_trigger_time_set(tsc_cfg.trigger_cnt);
    aic_tsc_debounce_cnt_set(tsc_cfg.debounce_cnt);
    aic_tsc_pre_chg_time_set(tsc_cfg.pre_chg_cnt);

#if TSC_DEBUG
    aicx_info(0x0692b4, "sd", "%s %d\n", __func__, __LINE__);
#endif

    register_pmic_irq(PMIC_IRQ_TSC, aic_tsc_irq_handler, (void *)NULL);
    aic_tsc_int_enabled(true);
    aic_tsc_sample_enabled(true);
    pmic_gpio9_f0tof2();

    g_p_tsc_sema = aos_create_semaphore("tsc semaphore", 0);
    if (AOS_NULL == g_p_tsc_sema) {
        aicx_error(0x066f2a, "", "tsc create semaphore fail \n");
        return AIC_EPERM;
    }

    /* create thread to check point release event */
    g_tsc_thread = aos_create_thread("tsc_thread",
                                        tsc_thread_entry,
                                        NULL, TSC_STACK_SIZE,
                                        0, NULL, TSC_TASK_PRIORITY,
                                        AOS_AUTO_START);

    ret = register_backlight_switch_monitor("tsc_adc", bl_bound_adc_func);

    return ret;
}

void aic_tsc_irq_deinit(void)
{
    pmic_writel(REG_ADC_CTRL_INT_EN, 0);
    unregister_pmic_irq(PMIC_IRQ_TSC);
}

/*  Must after pmic_pin_init */
AIC_APP_L2_INIT_EXPORT(aic_tsc_irq_init);
