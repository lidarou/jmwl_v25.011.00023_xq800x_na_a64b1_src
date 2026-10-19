/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is device status light at cmd handle.
 *
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1bee6b
#define LOG_MODULE    AMOD_PMIC
#include <aic_osa.h>
#include <aic_type.h>
#include <aic_at_process_common.h>
#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_reg_base.h>
#include <aic_at_resp_handle.h>
#include <aic_bitmap.h>
#include <adbg_cfg_api.h>
#include <gpio_inc.h>
#include <status_light_cfg.h>
#include <slp_inc.h>
#include <pmu_user.h>
#include <ipu.h>
#include <eipu.h>

static aos_thread_id net_light_task_id = AOS_INVALID_THREAD_ID;
static int net_light_notice = 0;
static u32 light_duration = NET_REGISTERED_LIGHT_DURATION,
           light_off_duration = NET_REGISTERED_LIGHT_OFF_DURATION;

aos_timer_ptr net_light_timer = NULL;
aos_event_ptr netlight_cycle_event;
static bool net_light_eb = false;
#define CMD_EXE_TIMING  2
#define NET_LIGHT_TIMER_NAME "net_light_timer"
#define NETLIGHT_EVENT_RUN_FLAG     0x1

bool net_registered = false;

static int net_light_enable(bool enabled)
{
    int ret = 0;

    if (enabled)
        aic_gpio_output(NET_LIGHT_GPIO_NUM, GPIO_OUTPUT_LVL_HIGH);
    else
        aic_gpio_output(NET_LIGHT_GPIO_NUM, GPIO_OUTPUT_LVL_LOW);

    return ret;
}

void net_light_work_task(void *input)
{
    int ret;
    u32 event;

    while(1) {
        ret = aos_get_event(netlight_cycle_event, NETLIGHT_EVENT_RUN_FLAG, AOS_AND_CLEAR, &event, AOS_WAIT_FOREVER);
        if (ret)
            continue;

        net_light_enable(true);
        aos_sleep(light_duration);
        if (!net_registered)
            continue;

        net_light_enable(false);
        aos_sleep(light_off_duration);
    }
}

void net_light_work_action(void *param)
{
    aos_change_timer(net_light_timer, light_duration + light_off_duration + CMD_EXE_TIMING, AOS_NO_PERIOD);
    aos_set_event(netlight_cycle_event, NETLIGHT_EVENT_RUN_FLAG, AOS_OR);
}

static int net_light_task_enable(bool enabled)
{
    int ret = 0;

    if (enabled) {
        if (!net_light_notice) {
            aos_change_timer(net_light_timer, light_duration + light_off_duration + CMD_EXE_TIMING, AOS_NO_PERIOD);
            net_light_notice = 1;
        }
    } else{
        if (net_light_notice) {
            if (net_light_timer)
                aos_deactivate_timer(net_light_timer);
            net_light_notice = 0;
        }
    }

    return ret;
}

void deice_cereg_update(u8 cereg_state)
{
    if (cereg_state == 1 || cereg_state == 5) {
        net_registered = true;

    } else if (cereg_state == 0) {
        net_registered = false;
    } else {
        net_registered = false;
    }
}
void net_status_light_enable(bool enabled)
{
    int ret;
    ret = net_light_task_enable(enabled);
    if (!ret)
        net_light_eb = enabled;
}

int is_ap_wakeup(void)
{
    if ((AIC_READ32(REG_BASE_AON_IPU_MCU + 0x8) == INT_AON_EIPU)
        && ((AIC_READ32(REG_BASE_AON_EIPU+EIPU_EDGE_32K*0x40+0xc) & BIT(EXTINT_CP)) == BIT(EXTINT_CP)))
        return 0;
    else
        return 1;
}

void state_light_suspend(void *p)
{
    if (net_light_eb)
        net_light_task_enable(false);
}

void state_light_resume(void *p)
{
    if (!is_ap_wakeup())
        return;

    if (net_light_eb)
        net_light_task_enable(true);
}

int status_light_at_cmd_init(void)
{
    int ret = 0;

    if (aic_register_cereg_state_cb(deice_cereg_update)) {
        ret = AIC_ESRCH;
        goto out;
    }

    ret = aic_gpio_request(NET_LIGHT_GPIO_NUM);
    if (ret != AIC_SUCCESS) {
        ret = AT_ERROR_PARA_NOT_SUPPORT;
        goto out;
    }

    net_light_task_id = aos_create_thread("net_light_keep_task", net_light_work_task, AOS_NULL, 1024, 8,
              "net_light_keep_task", 12, AOS_AUTO_START);
    if (net_light_task_id == AOS_INVALID_THREAD_ID) {
        ret = AIC_EIO;
        goto out;
    }

    net_light_timer = aos_create_timer(NET_LIGHT_TIMER_NAME,
                                         net_light_work_action, NULL,
                                         light_duration + light_off_duration + CMD_EXE_TIMING,
                                         AOS_NO_ACTIVATE, AOS_NO_PERIOD);
    AIC_ASSERT(net_light_timer);

    netlight_cycle_event = aos_create_event("netlight_cycle_event");
    AIC_ASSERT(netlight_cycle_event);

    slp_register_sleep_hook(state_light_suspend, NULL, state_light_resume, NULL);

    net_status_light_enable(true);

out:
    return ret;
}
AIC_COMP_P1_INIT_EXPORT(status_light_at_cmd_init);
