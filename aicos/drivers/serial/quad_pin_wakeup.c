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
#define AIC_THIS_FILE    0x005129
#define LOG_MODULE    AMOD_DRV

#include <aic_osa.h>
#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_type.h>
#include <quad_pin_cfg.h>
#include <slp_inc.h>
#include <eipu.h>
#include <gpo_inc.h>
#include <aic_osa_ex.h>

#define ACK_WAKEUP_MCU             BIT_1
#define ACK_RELEASE_MCU_TIMER      BIT_2
#define ACK_RELEASE_MCU            BIT_0

static void quad_pin_ack_wakeup_modem_direct(u32 level)
{
    aic_gpo_output(mdc.ack_wakeup_modem, level);
}

static void quad_pin_wakeup_mcu_direct(u32 level)
{
    aic_gpo_output(mdc.wakeup_mcu, level);
}

static void quad_pin_wakeup_modem_eipu_callback(u32 id, u32 state,
                                                         u32 eipu_pol)
{
    u32 wakeup_status = mdc.wakeup_modem_level;

    mdc.wakeup_modem_level = !mdc.wakeup_modem_level;
    EIPU_SetPol(ND_MODE, EIPU_LATCH, mdc.wakeup_modem,
        mdc.wakeup_modem_level);

    aos_send_to_queue(mdc.wakeup_modem_q, &wakeup_status, AOS_NO_WAIT);
}

static void quad_pin_ack_wakeup_mcu_eipu_callback(u32 id, u32 state,
                                                         u32 eipu_pol)
{
    u32 event = mdc.ack_wakeup_mcu_level ? ACK_WAKEUP_MCU : ACK_RELEASE_MCU;

    /* drv_logi("ack_wakeup_mcu mdc.ack_wakeup_mcu_level = %d", mdc.ack_wakeup_mcu_level); */
    mdc.ack_wakeup_mcu_level = !mdc.ack_wakeup_mcu_level;
    EIPU_SetPol(ND_MODE, EIPU_LATCH, mdc.ack_wakeup_mcu,
        mdc.ack_wakeup_mcu_level);

    aos_set_event(mdc.event, event, AOS_OR);
}

static void quad_pin_release_target_mcu_sync(void)
{
    u32 actual_flags, event, status;

    event = ACK_RELEASE_MCU;
    quad_pin_wakeup_mcu_direct(0);
    status = aos_get_event(mdc.event, event, AOS_AND_CLEAR, &actual_flags,
                       AOS_WAIT_FOREVER);
    AIC_ASSERT(!status);
    mdc.mcu_status = M_SLEEP;
}

void quad_pin_release_target_mcu(void)
{
    aos_change_timer(mdc.release_timer, mdc.release_delay_time, AOS_NO_PERIOD);
}

int quad_pin_wakeup_target_mcu(u32 timeout_ms)
{
    u32 actual_flags, event, status;

    if (!timeout_ms)
       timeout_ms = AOS_WAIT_FOREVER;

    aos_deactivate_timer(mdc.release_timer);
    status = aos_get_mutex(mdc.mutex, AOS_WAIT_FOREVER);
    AIC_ASSERT(!status);
    if (mdc.mcu_status) {
        aos_put_mutex(mdc.mutex);
        return AOS_SUCCESS;
    }
    aos_put_mutex(mdc.mutex);

    event = ACK_WAKEUP_MCU;
    quad_pin_wakeup_mcu_direct(1);
    status = aos_get_event(mdc.event, event, AOS_AND_CLEAR, &actual_flags,
                       timeout_ms);
    if (status) {
        quad_pin_wakeup_mcu_direct(0);
        return status;
    }

    mdc.mcu_status = M_WAKEUP;
    return status;
}

static void quad_pin_wakeup_modem_entry(void *param)
{
    u32 ret, wakeup_status;

    while(1) {
        ret = aos_recv_from_queue(mdc.wakeup_modem_q, &wakeup_status,
                                  AOS_WAIT_FOREVER);
        if (ret)
            continue;

        if (mdc.modem_status == wakeup_status)
            continue;

        if (wakeup_status == M_WAKEUP)
            aic_get_wakelock();
        else
            aic_put_wakelock();

        mdc.modem_status = wakeup_status;
        quad_pin_ack_wakeup_modem_direct(wakeup_status);
    }
}

static void quad_pin_wakeup_mcu_entry(void *param)
{
    u32 actual_flags, event, status;

    while(1) {
        event = ACK_RELEASE_MCU_TIMER;
        status = aos_get_event(mdc.event, event, AOS_AND_CLEAR, &actual_flags,
                           AOS_WAIT_FOREVER);
        AIC_ASSERT(!status);

        status = aos_get_mutex(mdc.mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(!status);
        quad_pin_release_target_mcu_sync();
        aos_put_mutex(mdc.mutex);
    }
}

void gpio_release_target_mcu(void)
{
    quad_pin_release_target_mcu();
}

int gpio_wakeup_target_mcu(u32 timeout_ms)
{
    return quad_pin_wakeup_target_mcu(timeout_ms);
}

void quad_pin_release_mcu_timeout(void *param)
{
    u32 event = ACK_RELEASE_MCU_TIMER;

    aos_set_event(mdc.event, event, AOS_OR);
}

int quad_pin_wakeup_ack_mcu_init(void)
{
    mdc.event = aos_create_event("mcu_wakeup");
    AIC_ASSERT(mdc.event);

    mdc.mutex = aos_create_mutex("quad_pin_mutex");
    AIC_ASSERT(mdc.mutex);

    mdc.release_timer = aos_create_timer("release_mcu",
                                         quad_pin_release_mcu_timeout, NULL,
                                         mdc.release_delay_time,
                                         AOS_NO_ACTIVATE, AOS_NO_PERIOD);
    AIC_ASSERT(mdc.release_timer);

    mdc.wakeup_modem_q = aos_create_queue("modem_wakeup_queue", sizeof(u32),
                                        QUAD_PIN_TASK_QUEUE_CNT);
    AIC_ASSERT(AOS_NULL != mdc.wakeup_modem_q);

    mdc.modem_thread = aos_create_thread("modem_wakeup",
                                         quad_pin_wakeup_modem_entry, AOS_NULL,
                                         QUAD_PIN_TASK_STACK_SIZE, 0,
                                         AIC_NULL, QUAD_PIN_TASK_PRIORITY,
                                         AOS_AUTO_START);
    AIC_ASSERT(AOS_INVALID_THREAD_ID != mdc.modem_thread);

    mdc.mcu_thread = aos_create_thread("mcu_wakeup",
                                         quad_pin_wakeup_mcu_entry, AOS_NULL,
                                         QUAD_PIN_TASK_STACK_SIZE, 0,
                                         AIC_NULL, QUAD_PIN_TASK_PRIORITY,
                                         AOS_AUTO_START);
    AIC_ASSERT(AOS_INVALID_THREAD_ID != mdc.mcu_thread);

    EIPU_RegCallback(mdc.ack_wakeup_mcu, EIPU_LATCH, quad_pin_ack_wakeup_mcu_eipu_callback,
        mdc.ack_wakeup_mcu_level, 0, ND_MODE);
    EIPU_RegCallback(mdc.wakeup_modem, EIPU_LATCH, quad_pin_wakeup_modem_eipu_callback,
        mdc.wakeup_modem_level, 0, ND_MODE);


    return AIC_SUCCESS;
}

AIC_DRV_P2_INIT_EXPORT(quad_pin_wakeup_ack_mcu_init);

