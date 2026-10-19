/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 * This file use for the pin wakeup detect.
 */

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x17e68d
#define LOG_MODULE    AMOD_DRV

#include <aic_osa.h>
#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_type.h>
#include <gpio_wakeup_cfg.h>
#include <slp_inc.h>
#include <timer_inc.h>
#include <eipu.h>
#include <ext_modem.h>

static int gpio_indicate_get_pin_status(void)
{
    if (mctrl.eipu_mode)
        return EIPU_GetIndata(NOSEC_MODE, EIPU_EDGE_32K, mctrl.eipu);
    else
        return aic_gpio_level_get(mctrl.indicate_gpio);
}

static int gpio_wait_target_mcu_wakeup(u32 timeout)
{
    u64 timeout_tick;
    DEBUG_PRINTF("gpio_wait_target_mcu_wakeup\n");

    timeout_tick = aic_get_tick() + timeout;
    while (!gpio_indicate_get_pin_status()) {
        if (timeout && (timeout_tick < aic_get_tick())) {
            DEBUG_PRINTF("gpio_wait_target_mcu_wakeup: timeout\n");
            return -1;
        }
    }
    return 0;
}

void gpio_wait_target_mcu_wakeup_dir(void)
{
    /* wait mcu ready */
    while (!gpio_indicate_get_pin_status()) {
        /* try wakeup mcu. */
        aic_gpio_output(mctrl.wakeup_gpio, GPIO_OUTPUT_LVL_LOW);
        aic_udelay(1000);
        aic_gpio_output(mctrl.wakeup_gpio, GPIO_OUTPUT_LVL_HIGH);
        aic_udelay(1000*100);
    }
    /* keep high to response mcu */
    aic_gpio_output(mctrl.wakeup_gpio, GPIO_OUTPUT_LVL_HIGH);
}

static void gpio_release_target_mcu_direct(void)
{
    pin_output(mctrl.wakeup_gpio, 0);
    aic_udelay(mctrl.after_release_mcu_delay_us);
    DEBUG_PRINTF("gpio_release_target_mcu_direct\n");
}

static void gpio_wakeup_target_mcu_direct(void)
{
    pin_output(mctrl.wakeup_gpio, 1);
}

void gpio_release_target_mcu(void)
{
    AOS_INTERRUPT_DECLARE
    u32 mutex_status;

    DEBUG_PRINTF("gpio_release_target_mcu\n");

    mutex_status = aos_get_mutex(mctrl.mutex, AOS_WAIT_FOREVER);
    AIC_ASSERT(AOS_SUCCESS == mutex_status);
    aos_sleep(mctrl.before_release_mcu_delay_ms);
    AOS_INTERRUPT_DISABLE
    gpio_release_target_mcu_direct();
    mctrl.request = ACK_TRANSFER;
    mctrl.mcu_slp = MCU_SLEEP;
    AOS_INTERRUPT_RESTORE
    aos_put_mutex(mctrl.mutex);

    aic_put_wakelock();
}

int gpio_wakeup_target_mcu(u32 timeout_ms)
{
    AOS_INTERRUPT_DECLARE
    u32 mutex_status;
    int ret, i;

    for (i = 0; i < WAKEUP_MCU_TRY_TIMES; i++) {
        aic_get_wakelock();
        mutex_status = aos_get_mutex(mctrl.mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(AOS_SUCCESS == mutex_status);
        AOS_INTERRUPT_DISABLE
        mctrl.request = REQ_TRANSFER;
        pin_output(mctrl.wakeup_gpio, 1);
        if (mctrl.mcu_slp) {
            AOS_INTERRUPT_RESTORE
            aos_put_mutex(mctrl.mutex);
            return AOS_SUCCESS;
        }
        AOS_INTERRUPT_RESTORE
        aos_put_mutex(mctrl.mutex);
        DEBUG_PRINTF("gpio_wakeup_target_mcu\n");

        ret = gpio_wait_target_mcu_wakeup(timeout_ms);
        if (ret) {
            gpio_release_target_mcu_direct();
            aic_put_wakelock();
        }

        if (!ret)
            break;
    }
    return ret;
}

static void gpio_wakeup_mcu_callback(void *param)
{
    u32 gpio_level, wakeup_status;

    gpio_level = aic_gpio_level_get(mctrl.indicate_gpio);
    if (gpio_level) {
        wakeup_status = MCU_WAKEUP;
        mctrl.mcu_slp = MCU_WAKEUP;
        gpio_wakeup_target_mcu_direct();
    } else {
        wakeup_status = MCU_SLEEP;
        if (!mctrl.request && !gpio_indicate_get_pin_status()) {
            mctrl.mcu_slp = MCU_SLEEP;
            gpio_release_target_mcu_direct();
        }
    }

    DEBUG_PRINTF("gpio_wakeup_mcu_callback pin status: %s\n", wakeup_status ? "wakeup" : "sleep");
    aos_send_to_queue(mctrl.wakeup_mcu_q, &wakeup_status, AOS_NO_WAIT);
}


static void gpio_eipu_callback(u32 id, u32 state, u32 eipu_pol)
{
    u32 gpio_level, wakeup_status;

    gpio_level = EIPU_GetIndata(NOSEC_MODE, EIPU_EDGE_32K, id);
    EIPU_SetPol(NOSEC_MODE, EIPU_LATCH, mctrl.eipu, !gpio_level);

    if (gpio_level) {
        wakeup_status = MCU_WAKEUP;
        mctrl.mcu_slp = MCU_WAKEUP;
        gpio_wakeup_target_mcu_direct();
    } else {
        wakeup_status = MCU_SLEEP;
        if (!mctrl.request && !gpio_indicate_get_pin_status()) {
            mctrl.mcu_slp = MCU_SLEEP;
            gpio_release_target_mcu_direct();
        }
    }

    DEBUG_PRINTF("gpio_wakeup_mcu_callback pin status: %s\n", wakeup_status ? "wakeup" : "sleep");
    aos_send_to_queue(mctrl.wakeup_mcu_q, &wakeup_status, AOS_NO_WAIT);
}


static void gpio_wakeup_mcu_entry(void *param)
{
    u32 ret, wakeup_status;
    u32 mutex_status;

    while(1) {
        ret = aos_recv_from_queue(mctrl.wakeup_mcu_q, &wakeup_status,
                                  AOS_WAIT_FOREVER);
        if (ret)
            continue;

        if (wakeup_status == mctrl.modem_slp)
            continue;

        mutex_status = aos_get_mutex(mctrl.mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(AOS_SUCCESS == mutex_status);
        DEBUG_PRINTF("wakeup pin status: %s\n", wakeup_status ? "wakeup" : "sleep");
        if (wakeup_status == MCU_WAKEUP) {
            mctrl.modem_slp = MODEM_WAKEUP;
            aic_get_wakelock();
        } else {
            mctrl.modem_slp = MODEM_SLEEP;
            aic_put_wakelock();
        }
        aos_put_mutex(mctrl.mutex);
    }
}

void gpio_wakeuo_config_init(void)
{
    struct ext_modem_gpio_wakeup * gpio_wakeup_cfg = ext_modem_get_gpio_wakeup_cfg();

    if (!gpio_wakeup_cfg)
        return;

    mctrl.wakeup_gpio = gpio_wakeup_cfg->wakeup_mcu_n;
    mctrl.indicate_gpio = gpio_wakeup_cfg->wakeup_gx_n;
    mctrl.eipu_mode = gpio_wakeup_cfg->wakeup_type;
    mctrl.eipu = mctrl.indicate_gpio;

    AIC_WRITE32(gpio_wakeup_cfg->wakeup_gx_iomuxaddr, gpio_wakeup_cfg->wakeup_gx_iomuxval);
    AIC_WRITE32(gpio_wakeup_cfg->wakeup_gx_pinaddr, gpio_wakeup_cfg->wakeup_gx_pinval);
    AIC_WRITE32(gpio_wakeup_cfg->wakeup_mcu_iomuxaddr, gpio_wakeup_cfg->wakeup_mcu_iomuxval);
    AIC_WRITE32(gpio_wakeup_cfg->wakeup_mcu_pinaddr, gpio_wakeup_cfg->wakeup_mcu_pinval);
}

/**
 * aic_pin_wakeup_init - wakeup by pin.
 *
 * Return:
 *
 */
int gpio_wakeup_mcu_init(void)
{
    int ret;

    gpio_wakeuo_config_init();
    mctrl.wakeup_mcu = aos_create_thread("wakeup_mcu",
                                             gpio_wakeup_mcu_entry, AOS_NULL,
                                             GPIO_WAKEUP_TASK_STACK_SIZE, 0,
                                             AIC_NULL, GPIO_WAKEUP_TASK_PRIORITY,
                                             AOS_AUTO_START);
    AIC_ASSERT(AOS_INVALID_THREAD_ID != mctrl.wakeup_mcu);

    mctrl.wakeup_mcu_q = aos_create_queue("wakeup_mcu_queue", sizeof(u32),
                                        GPIO_WAKEUP_TASK_QUEUE_CNT);
    AIC_ASSERT(AOS_NULL != mctrl.wakeup_mcu_q);

    mctrl.mutex = aos_create_mutex("gpio wakeup");
    AIC_ASSERT(mctrl.mutex);

    pin_output(mctrl.wakeup_gpio, 0);

    if (mctrl.eipu_mode) {
        EIPU_RegCallback(mctrl.eipu, EIPU_LATCH, gpio_eipu_callback, HIGH_LEVEL, 0, NOSEC_MODE);
    } else {
        ret = aic_gpio_request(mctrl.indicate_gpio);
        AIC_ASSERT(ret == AIC_SUCCESS);
        aic_gpio_input(mctrl.indicate_gpio, GPIO_TRIGGER_EDGE_ANY,
                       gpio_wakeup_mcu_callback, NULL);
    }

    return AIC_SUCCESS;
}

AIC_DRV_P2_INIT_EXPORT(gpio_wakeup_mcu_init);

