/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is lamp torch led at cmd handle.
 *
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0a7649
#define LOG_MODULE    AMOD_PMIC
#include <aic_osa.h>
#include <aic_type.h>
#include <aic_at_process_common.h>
#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_reg_base.h>
#include <aic_at_resp_handle.h>
#include <aic_bitmap.h>
#include <cache.h>
#include <pmu_user.h>
#include <timer_inc.h>

#define DISPLAY_MAX_AT_RESP_LENGTH        64UL
#define DISPLAY_REPLY_LINE_COUNT_ONCE     2UL

#define LED_MAX_CURRENT     30000

struct pmu_user *led_r_handle = NULL;
struct pmu_user *led_g_handle = NULL;
struct pmu_user *led_b_handle = NULL;
struct pmu_user *led_w_handle = NULL;

static u32 at_led_on_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];
    int len;

    strcpy(rsp_str, "^LEDON");
    len = strlen(rsp_str);
    rsp_str[len++] = '=';
    sprintf(rsp_str + len, "R/G/B/W.(RED/GREEN/BLUE/WHITE)");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 at_led_on_cmd(AT_REQ_RESP *at_req)
{
    struct pmu_user *pled;
    ulong *paras_arr;
    ulong rsp[1];
    int ret = 0;
    u16 count;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^LEDON=", &count);
    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    if (!strcmp((char *)paras_arr[0],"R") || !strcmp((char *)paras_arr[0],"r"))
        pled = led_r_handle;
    else if (!strcmp((char *)paras_arr[0],"G") || !strcmp((char *)paras_arr[0],"g"))
        pled = led_g_handle;
    else if (!strcmp((char *)paras_arr[0],"B") || !strcmp((char *)paras_arr[0],"b"))
        pled = led_b_handle;
    else if (!strcmp((char *)paras_arr[0],"W") || !strcmp((char *)paras_arr[0],"w"))
        pled = led_w_handle;
    else
        ret = AIC_ENXIO;

    if (ret) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    ret = pmu_enable(pled);
    if (ret)
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else
        rsp[0] = (ulong)AT_RESULT_OK;

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

static u32 at_led_off_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];
    int len;

    strcpy(rsp_str, "^LEDOFF");
    len = strlen(rsp_str);
    rsp_str[len++] = '=';
    sprintf(rsp_str + len, "R/G/B/W.(RED/GREEN/BLUE/WHITE)");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 at_led_off_cmd(AT_REQ_RESP *at_req)
{
    struct pmu_user *pled;
    ulong *paras_arr;
    int ret = 0;
    ulong rsp[1];
    u16 count;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^LEDOFF=", &count);
    if (!paras_arr || count != 1) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    if (!strcmp((char *)paras_arr[0],"R") || !strcmp((char *)paras_arr[0],"r"))
        pled = led_r_handle;
    else if (!strcmp((char *)paras_arr[0],"G") || !strcmp((char *)paras_arr[0],"g"))
        pled = led_g_handle;
    else if (!strcmp((char *)paras_arr[0],"B") || !strcmp((char *)paras_arr[0],"b"))
        pled = led_b_handle;
    else if (!strcmp((char *)paras_arr[0],"W") || !strcmp((char *)paras_arr[0],"w"))
        pled = led_w_handle;
    else
        ret = AIC_ENXIO;

    if (ret) {
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
        goto AT_REPLY;
    }

    ret = pmu_disable(pled);
    if (ret)
        rsp[0] = (ulong)AT_SIMPLE_RESULT_ERROR;
    else
        rsp[0] = (ulong)AT_RESULT_OK;

AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

AT_CMD_HANDLE led_at_handles[] = {
    {
        "AT^LEDON",
        at_led_on_test_cmd,
        AIC_NULL,
        at_led_on_cmd,
        AIC_NULL
    },
    {
        "AT^LEDOFF",
        at_led_off_test_cmd,
        AIC_NULL,
        at_led_off_cmd,
        AIC_NULL
    },
    {AIC_NULL},
};

int led_ops_init()
{
    int ret;

    led_r_handle = pmu_get("led_at", "LED_R");
    if (!led_r_handle) {
        ret = AIC_ENODEV;
        goto out;
    }

    led_g_handle = pmu_get("led_at", "LED_G");
    if (!led_g_handle) {
        ret = AIC_ENODEV;
        goto out;
    }

    led_b_handle = pmu_get("led_at", "LED_B");
    if (!led_b_handle) {
        ret = AIC_ENODEV;
        goto out;
    }

    led_w_handle = pmu_get("led_at", "LED_W");
    if (!led_w_handle) {
        ret = AIC_ENODEV;
        goto out;
    }

    ret = pmu_current_set(led_r_handle, LED_MAX_CURRENT, 800);
    if (ret) {
        ret = AIC_ENODEV;
        goto out;
    }

    ret = pmu_current_set(led_g_handle, LED_MAX_CURRENT, 800);
    if (ret) {
        ret = AIC_ENODEV;
        goto out;
    }

    ret = pmu_current_set(led_b_handle, LED_MAX_CURRENT, 800);
    if (ret) {
        ret = AIC_ENODEV;
        goto out;
    }

    ret = pmu_current_set(led_w_handle, LED_MAX_CURRENT, 800);
    if (ret)
        ret = AIC_ENODEV;

out:
    return ret;
}

int led_at_cmd_init(void)
{
    int ret;

    ret = led_ops_init();
    if (ret)
        return AIC_ENOENT;

    aic_add_at_cmd_handle(&led_at_handles[0]);

    return 0;
}
AIC_COMP_L2_INIT_EXPORT(led_at_cmd_init);
