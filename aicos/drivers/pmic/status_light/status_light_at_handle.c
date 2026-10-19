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
#define AIC_THIS_FILE    0x172258
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

#if defined(DEVICE_SLEEP_STATUS_LIGHT_GPO_NUM) || defined(DEVICE_STATUS_LIGHT_GPO_NUM) || defined(NET_LIGHT_GPO_NUM)
#include <gpo_inc.h>
#endif

static aos_thread_id net_light_task_id = AOS_INVALID_THREAD_ID;
static int net_light_notice = 0, device_state_notice = 0;
static u32 light_duration = NET_NOT_REGISTERED_LIGHT_DURATION,
            light_off_duration = NET_NOT_REGISTERED_LIGHT_OFF_DURATION;

aos_timer_ptr net_light_timer = NULL;
aos_event_ptr netlight_cycle_event;
static bool net_light_eb = false;
#define CMD_EXE_TIMING  2
#define NET_LIGHT_TIMER_NAME "net_light_timer"
#define NETLIGHT_EVENT_RUN_FLAG     0x1

#ifdef NET_LIGHT_LED_NAME
struct pmu_user *nl_led_handle = NULL;
#endif

#ifdef DEVICE_STATUS_LIGHT_LED_NAME
struct pmu_user *ds_led_handle = NULL;
#endif

static int net_light_enable(bool enabled)
{
    int ret = 0;

#ifdef NET_LIGHT_GPIO_NUM
    if (enabled)
        aic_gpio_output(NET_LIGHT_GPIO_NUM, GPIO_OUTPUT_LVL_HIGH);
    else
        aic_gpio_output(NET_LIGHT_GPIO_NUM, GPIO_OUTPUT_LVL_LOW);
#elif defined(NET_LIGHT_GPO_NUM)
    if (enabled)
        aic_gpo_output(NET_LIGHT_GPO_NUM, GPO_OUTPUT_HIGH);
    else
        aic_gpo_output(NET_LIGHT_GPO_NUM, GPO_OUTPUT_LOW);

#elif defined(NET_LIGHT_LED_NAME)
    if (enabled)
        ret = pmu_enable(nl_led_handle);
    else
        ret = pmu_force_disable(nl_led_handle);
#endif
    return ret;
}

static int device_state_light_enable(bool enabled)
{
    int ret = 0;
#ifdef DEVICE_STATUS_LIGHT_GPIO_NUM
    if (enabled)
        aic_gpio_output(DEVICE_STATUS_LIGHT_GPIO_NUM, GPIO_OUTPUT_LVL_HIGH);
    else
        aic_gpio_output(DEVICE_STATUS_LIGHT_GPIO_NUM, GPIO_OUTPUT_LVL_LOW);
#elif defined(DEVICE_STATUS_LIGHT_GPO_NUM)
    if (enabled)
        aic_gpo_output(DEVICE_STATUS_LIGHT_GPO_NUM, GPO_OUTPUT_HIGH);
    else
        aic_gpo_output(DEVICE_STATUS_LIGHT_GPO_NUM, GPO_OUTPUT_LOW);
#elif defined(DEVICE_STATUS_LIGHT_LED_NAME)
    if (enabled)
        ret = pmu_enable(ds_led_handle);
    else
        ret = pmu_force_disable(ds_led_handle);
#endif

    if (!ret) {
        if (enabled)
            device_state_notice = 1;
        else
            device_state_notice = 0;
    }

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

static u32 at_mled_on_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];

    sprintf(rsp_str, "+MLED: (0-1),(0-1)");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 at_mled_on_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u8 result_line1[5] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    ulong *paras_arr;
    u16 count;
    int type, state, ret;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+MLED=", &count);
    if (!paras_arr)
        goto CMD_ERROR;

    if (count == 1) {
        type = atoi((u8 *)paras_arr[0]);

        if (type != 0 && type != 1)
            goto CMD_ERROR;

        snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "+MLED: %d,%d", type, type?device_state_notice:net_light_notice);
        strncpy(result_line1, AT_RESULT_OK, (sizeof(result_line1) - 1));
        /* Fill response,two lines. */
        line_pointers[0] = (ulong)&result_line0[0];
        line_pointers[1] = (ulong)&result_line1[0];
        aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);
    } else if (count ==2) {
        type = atoi((u8 *)paras_arr[0]);
        state = atoi((u8 *)paras_arr[1]);

        if (state !=0 && state != 1)
            goto CMD_ERROR;

        if (type == 0) {
            ret = net_light_task_enable(state);
            if (!ret)
                net_light_eb = state;
        } else if (type == 1)
            ret = device_state_light_enable(state);
        else
            goto CMD_ERROR;

        if (ret)
            goto CMD_ERROR;

        adbg_set_status_light(type, state);

        strcpy(result_line0, "OK");
        /* fill response,one lines. */
        line_pointers[0] = (ulong)&result_line0[0];
        aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    } else {
        goto CMD_ERROR;
    }

    return AIC_SUCCESS;

CMD_ERROR:
    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "+CME ERROR: %d", AT_ERROR_INVALID_CMD);
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

AT_CMD_HANDLE status_light_at_handles[] = {
    {
        "AT+MLED",
        at_mled_on_test_cmd,
        AIC_NULL,
        at_mled_on_cmd,
        AIC_NULL
    },
    {AIC_NULL},
};
void deice_cereg_update(u8 cereg_state)
{
    if (cereg_state == 1 || cereg_state == 5) {
        light_duration = NET_REGISTERED_LIGHT_DURATION;
        light_off_duration = NET_REGISTERED_LIGHT_OFF_DURATION;
    } else if (cereg_state == 0) {
        light_duration = NET_NOT_REGISTERED_LIGHT_DURATION;
        light_off_duration = NET_NOT_REGISTERED_LIGHT_OFF_DURATION;
    } else {
        light_duration = NET_NOT_REGISTERED_LIGHT_DURATION;
        light_off_duration = NET_NOT_REGISTERED_LIGHT_OFF_DURATION;
    }
}

static int status_light_nv_init(void)
{
    int ret = 0;

    /*net light status init*/
    if (adbg_is_status_light(0)) {
        ret = net_light_task_enable(true);
        if (!ret)
            net_light_eb = true;
    }

    /*status light status init*/
    if (adbg_is_status_light(1))
        ret |= device_state_light_enable(true);

    return ret;
}

#ifdef CONFIG_SLEEP_R1P0
int is_ap_wakeup(void)
{
    if ((AIC_READ32(REG_BASE_AON_IPU_MCU + 0x8) == INT_AON_EIPU)
        && ((AIC_READ32(REG_BASE_AON_EIPU+EIPU_EDGE_32K*0x40+0xc) & BIT(EXTINT_CP)) == BIT(EXTINT_CP)))
        return 0;
    else
        return 1;
}
#else
int is_ap_wakeup(void)
{
    if ((AIC_READ32(REG_BASE_TOP_IPU_MCU + 0x8) == INT_REQ_AON_EIPU)
        && ((AIC_READ32(REG_BASE_TOP_AON_EIPU+EIPU_EDGE_32K*0x40+0xc) & BIT(5)) == BIT(5))
        && (AIC_READ32(REG_BASE_SYS_ND_GLB + 0x208) == 0))
        return 0;
    else
        return 1;
}
#endif

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

#ifdef DEVICE_SLEEP_STATUS_LIGHT_GPIO_NUM
void sleep_state_light_suspend(void *p)
{
    aic_gpio_output(DEVICE_SLEEP_STATUS_LIGHT_GPIO_NUM, GPIO_OUTPUT_LVL_LOW);
}

void sleep_state_light_resume(void *p)
{
    if (is_ap_wakeup())
        aic_gpio_output(DEVICE_SLEEP_STATUS_LIGHT_GPIO_NUM, GPIO_OUTPUT_LVL_HIGH);
}
#elif defined(DEVICE_SLEEP_STATUS_LIGHT_GPO_NUM)
void sleep_state_light_suspend(void *p)
{
    aic_gpo_output(DEVICE_SLEEP_STATUS_LIGHT_GPO_NUM, GPO_OUTPUT_LOW);
}

void sleep_state_light_resume(void *p)
{
    if (is_ap_wakeup())
        aic_gpo_output(DEVICE_SLEEP_STATUS_LIGHT_GPO_NUM, GPO_OUTPUT_HIGH);
}
#endif

int status_light_at_cmd_init(void)
{
    int ret = 0;

    aic_add_at_cmd_handle(&status_light_at_handles[0]);

    if (aic_register_cereg_state_cb(deice_cereg_update)) {
        ret = AIC_ESRCH;
        goto out;
    }

#ifdef NET_LIGHT_GPIO_NUM
    ret = aic_gpio_request(NET_LIGHT_GPIO_NUM);
    if (ret != AIC_SUCCESS) {
        ret = AT_ERROR_PARA_NOT_SUPPORT;
        goto out;
    }
#elif defined(NET_LIGHT_LED_NAME)
    nl_led_handle = pmu_get("status_light", NET_LIGHT_LED_NAME);
    if (!nl_led_handle) {
        ret = AIC_ENODEV;
        goto out;
    }
#endif

#ifdef DEVICE_STATUS_LIGHT_GPIO_NUM
    ret = aic_gpio_request(DEVICE_STATUS_LIGHT_GPIO_NUM);
    if (ret != AIC_SUCCESS) {
        ret = AT_ERROR_PARA_NOT_SUPPORT;
        goto out;
    }
#elif defined(DEVICE_STATUS_LIGHT_LED_NAME)
    ds_led_handle = pmu_get("status_light", DEVICE_STATUS_LIGHT_LED_NAME);
    if (!ds_led_handle) {
        ret = AIC_ENODEV;
        goto out;
    }
#endif

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

    ret = status_light_nv_init();

    slp_register_sleep_hook(state_light_suspend, NULL, state_light_resume, NULL);

#ifdef DEVICE_SLEEP_STATUS_LIGHT_GPIO_NUM
    ret = aic_gpio_request(DEVICE_SLEEP_STATUS_LIGHT_GPIO_NUM);
    if (ret != AIC_SUCCESS) {
        ret = AT_ERROR_PARA_NOT_SUPPORT;
        goto out;
    }

    aic_gpio_output(DEVICE_SLEEP_STATUS_LIGHT_GPIO_NUM, GPIO_OUTPUT_LVL_HIGH);

    slp_register_sleep_hook(sleep_state_light_suspend, NULL, sleep_state_light_resume, NULL);
#elif defined(DEVICE_SLEEP_STATUS_LIGHT_GPO_NUM)
    aic_gpo_output(DEVICE_SLEEP_STATUS_LIGHT_GPO_NUM, GPO_OUTPUT_HIGH);

    slp_register_sleep_hook(sleep_state_light_suspend, NULL, sleep_state_light_resume, NULL);
#endif

out:
    return ret;
}
AIC_COMP_L2_INIT_EXPORT(status_light_at_cmd_init);
