/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file backlight.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <aic_osa_ex.h>
#include "aic_srv_tmr.h"
#include "aic_srv_backlight.h"
#include "display.h" /* TODO: del this*/
#include "backlight.h"
#include "app_sleep.h"

/*********************
 *      DEFINES
 *********************/
#define BRIGHTNESS_MAX_OFFSET       (4)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

static srv_tmr_handle g_p_backlight_timer = NULL;

/* TODO: delete after backlight_set_config support*/
#ifdef CONFIG_FP
static backlight_config_t g_backlight_cfg = {40000, 0, 80, 20};
#else
static backlight_config_t g_backlight_cfg = {30000, 0, 100, 20};
#endif
static bool g_is_always_on = false;
/**********************
 *   STATIC FUNCTIONS
 **********************/
static int32_t backlight_timer_handle(void *p_param, uint32_t size)
{
    int ret_b = 0;
    uint32_t value = 0;
    srv_tmr_param_t backlight_timer_param = {0};
    backlight_timer_param.time_out = g_backlight_cfg.half_off_timeout;
    backlight_timer_param.is_repeat = 0;
    backlight_timer_param.p_name = "backlight timer";
    backlight_timer_param.p_usr_data = NULL;

    aic_bl_brightness_get((u32*)&value);

    if (abs(value - g_backlight_cfg.brightness_on) < BRIGHTNESS_MAX_OFFSET) {
        if (0 == g_backlight_cfg.half_off_timeout) {
            backlight_turn_off();
            app_logi("[%s] brightness from %d to 0, backlight turn off", __func__, g_backlight_cfg.brightness_on);
        } else {
            aic_srv_tmr_change(g_p_backlight_timer, &backlight_timer_param);
            ret_b = aic_srv_backlight_turn_on(g_backlight_cfg.brightness_half_on);
            app_logi("[%s] brightness is %d, return %d", __func__, g_backlight_cfg.brightness_half_on, ret_b);
        }
    } else if (abs(value - g_backlight_cfg.brightness_half_on) < BRIGHTNESS_MAX_OFFSET) {
        backlight_turn_off();
        app_logi("[%s] brightness from %d to 0, backlight turn off", __func__, g_backlight_cfg.brightness_half_on);
    } else {
        app_loge("[%s] brightness {%d, %d, %d} offset excessively", __func__, value,
            g_backlight_cfg.brightness_on, g_backlight_cfg.brightness_half_on);
    }

    return ret_b;
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
int backlight_set_config(backlight_config_t config)
{
    int ret_b =0;

    g_backlight_cfg.turn_off_timeout = config.turn_off_timeout;
    g_backlight_cfg.half_off_timeout = config.half_off_timeout;

    /*TODO : Delete relevant brightness settings judgement after the backlight scheme is updated*/
    /*NOTE : ensure the brightness set succeed and then trans*/
    ret_b =aic_bl_brightness_set(config.brightness_on);
    if(ret_b == 0) {
        g_backlight_cfg.brightness_on = config.brightness_on;
        app_logi("[%s] set brightness %d succeed", __func__, g_backlight_cfg.brightness_on);
    } else {
        aic_bl_brightness_set(g_backlight_cfg.brightness_on);
        app_logi("[%s] set brightness %d failed", __func__, config.brightness_on);
    }

    g_backlight_cfg.brightness_half_on = config.brightness_half_on;

    app_logi("[%s] backlight configuration is set to {%d, %d, %d, %d}", __func__, g_backlight_cfg.turn_off_timeout,
        g_backlight_cfg.half_off_timeout, g_backlight_cfg.brightness_on, g_backlight_cfg.brightness_half_on);

    /* TODO: Add Macro for turn on always */
    if(g_backlight_cfg.turn_off_timeout == UINT32_MAX) {
        /*Turn on always if turn_off_timeout is the MAX value*/
        backlight_turn_on_always();
    } else {
        /*Ensure exit on always, backlight turn on normal*/
        backlight_exit_turn_on_always();
    }
    return ret_b;
}

int backlight_get_config(backlight_config_t *p_config)
{
    if(NULL == p_config) {
        app_loge("[%s] the pointer is null", __func__);
        return -1;
    }
    p_config->turn_off_timeout = g_backlight_cfg.turn_off_timeout;
    p_config->half_off_timeout = g_backlight_cfg.half_off_timeout;
    p_config->brightness_on = g_backlight_cfg.brightness_on;
    p_config->brightness_half_on = g_backlight_cfg.brightness_half_on;
    app_logi("[%s] got backlight configuration is {%d, %d, %d, %d}", __func__,
        p_config->turn_off_timeout, p_config->half_off_timeout, p_config->brightness_on, p_config->brightness_half_on);

    return 0;
}
#ifdef CONFIG_FP
extern uint8_t get_auto_lock_status(void);
int backlight_turn_off1(void)
{
    int ret_b = 0;
    g_is_always_on = false;

    if (NULL != g_p_backlight_timer) {
        aic_srv_tmr_destroy(g_p_backlight_timer);
        g_p_backlight_timer = NULL;
    }
    ret_b = aic_srv_backlight_turn_off();
    /* current is backlight on, need enter sleep */
    /* NOTE: enter app sleep after backlight turn off */
    app_enter_sleep();

    return ret_b;
}
#endif

int backlight_turn_off(void)
{
    int ret_b = 0;
    g_is_always_on = false;

    if (NULL != g_p_backlight_timer) {
        aic_srv_tmr_destroy(g_p_backlight_timer);
        g_p_backlight_timer = NULL;
    }
    ret_b = aic_srv_backlight_turn_off();
#ifdef CONFIG_FP
    get_auto_lock_status();
#endif
    /* current is backlight on, need enter sleep */
    /* NOTE: enter app sleep after backlight turn off */
    app_enter_sleep();

    return ret_b;
}

int backlight_turn_on(void)
{
    int ret_b = 0;
    uint32_t current_value = 0;
    uint32_t turn_half_timeout = 0;

    if(true == g_is_always_on) {
        app_logi("[%s] backlight turn on always, return", __func__);
        return ret_b;
    }
    turn_half_timeout = g_backlight_cfg.turn_off_timeout - g_backlight_cfg.half_off_timeout;

    aic_bl_brightness_get((u32*)&current_value);

    srv_tmr_param_t backlight_timer_param = {0};
    backlight_timer_param.time_out = turn_half_timeout;
    backlight_timer_param.is_repeat = 0;
    backlight_timer_param.p_name = "backlight timer";
    backlight_timer_param.p_usr_data = NULL;

    if (NULL == g_p_backlight_timer) {
        g_p_backlight_timer = aic_srv_tmr_create( &backlight_timer_param, backlight_timer_handle);
        app_logi("[%s] create a backlight timer for %d s", __func__, turn_half_timeout/1000);
    } else {
        aic_srv_tmr_change(g_p_backlight_timer, &backlight_timer_param);
        app_logi("[%s] the backlight timer is changed to %d s", __func__, turn_half_timeout/1000);
    }

    if (0 == current_value) {
        app_logi("[%s] backlight off to on", __func__);
        /* current is backlight off, need exit sleep first */
        /* NOTE: need exit app sleep before backlight turn on */
        app_exit_sleep();
    }

    ret_b = aic_srv_backlight_turn_on(g_backlight_cfg.brightness_on);
    app_logi("[%s] brightness is %d,return %d", __func__, g_backlight_cfg.brightness_on,ret_b);
    return ret_b;
}

int backlight_turn_on_always(void)
{
    int ret_b = 0;
    g_is_always_on = true;

    if (NULL != g_p_backlight_timer) {
        aic_srv_tmr_destroy(g_p_backlight_timer);
        g_p_backlight_timer = NULL;
        app_logi("[%s] timer is exist, backlight on to on always, destroy timer", __func__);
    } else {
        /* TODO: Add Macro for turn on always */
        if(UINT32_MAX == g_backlight_cfg.turn_off_timeout) {
            app_logi("[%s] timer is not exist, backlight already on always, return directly", __func__);
            return ret_b;
        }
        app_exit_sleep();
        app_logi("[%s] timer is not exist, backlight off to on always, app exit sleep", __func__);
    }
    ret_b = aic_srv_backlight_turn_on(g_backlight_cfg.brightness_on);

    return ret_b;
}
int backlight_exit_turn_on_always(void)
{
    int ret_b = 0;
    if(UINT32_MAX == g_backlight_cfg.turn_off_timeout) {
        app_logi("[%s] turn off timeout is UINT32_MAX, keep backlight on always, don't restart timer, return directly", __func__);
        return ret_b;
    }
    g_is_always_on = false;

    ret_b = backlight_turn_on();
    app_logi("exit backlight on always, backlight turn on normal, return %d!", ret_b);

    return ret_b;
}

#include <aic_at_process_common.h>
#include <aic_at_resp_handle.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <adbg_cfg_api.h>

#define QUICK_SCREEN_OFF_TIME   1000

static u32 at_appslp_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];

    sprintf(rsp_str, "+appslp: (auto sleep time unit ms)");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static int set_backlight_timer(u32 timeout_ms)
{
    int ret;
    backlight_config_t cfg = {0};

    backlight_get_config(&cfg);
    cfg.turn_off_timeout = timeout_ms;
    ret = backlight_set_config(cfg);

    return ret;
}

static u32 at_appslp_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    ulong *paras_arr;
    u16 count;
    int ret, app_timeout;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+APPSLP=", &count);
    if (!paras_arr || count != 1)
        goto CMD_ERROR;

    app_timeout = atoi((u8 *)paras_arr[0]);

    ret = set_backlight_timer(app_timeout);
    if (ret)
        goto CMD_ERROR;

    if (app_timeout <= QUICK_SCREEN_OFF_TIME)
        adbg_set_qs_off(1);
    else
        adbg_set_qs_off(0);

    strcpy(result_line0, "OK");
    /* fill response,one lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    aic_free_paras_mem(paras_arr, count);
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    return AIC_SUCCESS;

CMD_ERROR:
    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "+CME ERROR: %d", AT_ERROR_INVALID_CMD);
    line_pointers[0] = (ulong)&result_line0[0];
    aic_free_paras_mem(paras_arr, count);
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

AT_CMD_HANDLE appslp_at_handles[] = {
    {
        "AT+APPSLP",
        at_appslp_test_cmd,
        AIC_NULL,
        at_appslp_cmd,
        AIC_NULL
    },
    {AIC_NULL},
};

int app_slp_cmd_init(void)
{
    aic_add_at_cmd_handle(&appslp_at_handles[0]);

    if (adbg_is_qs_off())
        set_backlight_timer(QUICK_SCREEN_OFF_TIME);

    return 0;
}
AIC_COMP_L2_INIT_EXPORT(app_slp_cmd_init);
