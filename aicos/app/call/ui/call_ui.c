/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is enter of call
 */
/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "stdio.h"

#include "alv_app_mgr.h"
#include "statusbar.h"
#include "softkey.h"
#include "call_flow.h"

#ifdef CONFIG_SUPPORT_PHYSICAL_KEY
#include "win_dialer_withkey/ui.h"
#else
#include "win_dialer/ui.h"
#endif
#include "win_connected/ui.h"
#include "win_outgoing/ui.h"
#include "win_incoming/ui.h"


/*********************
 *      DEFINES
 *********************/
#ifndef min
#define min(a, b)  ((a) < (b) ? (a) : (b))
#endif

#define CALL_APP_NAME                      "call"
#define CALL_WIN_DIAL_NAME                 "call_dial"
#define CALL_WIN_OUTGOING_NAME             "call_outgoing"
#define CALL_WIN_CONNECT_NAME              "call_connected"
#define CALL_WIN_INCOMING_NAME             "call_incoming"

#define CALL_APP_NAME_LEN                  (5)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

call_origin_e g_call_origin = CALL_ORIGIN_DIALDER;
/* dialer win obj */
static lv_obj_t *g_call_dialer_win = NULL;
/* incoming win obj */
static lv_obj_t *g_call_incoming_win = NULL;
/* outgoing win obj */
static lv_obj_t *g_call_outgoing_win = NULL;


/**********************
 *   STATIC FUNCTIONS
 **********************/
static int32_t win_incoming_show(call_enter_param_t incoming_info);
static int32_t win_connected_show(uint8_t call_idx, char *p_phone_num);
static int32_t win_released_show(uint32_t reason);
static int32_t ims_status_show(bool ims_on);

const call_flow_t g_call_wins_flow = {
    win_incoming_show,
    win_connected_show,
    win_released_show,
    ims_status_show,
};

static int32_t win_incoming_show(call_enter_param_t incoming_info)
{
    if(false == alv_am_app_is_exist("call")) {
        call_enter_param_t enter_param = {0};
        enter_param.call_origin = CALL_ORIGIN_GLOBAL_INCOMING;
        strncpy(enter_param.phone_num, incoming_info.phone_num, TS_CALL_PHONE_NUMBER_LEN_MAX);
        alv_am_app_enter(NULL, "call", &enter_param);
    } else {
        alv_am_win_enter(NULL, CALL_WIN_INCOMING_NAME, incoming_info.phone_num);
    }

    return 0;
}

static int32_t win_connected_show(uint8_t call_idx, char *p_phone_num)
{
    if(false == alv_am_app_is_exist(CALL_APP_NAME)) {
        LV_LOG_ERROR("[%s] TS_CALL_STATUS_ACTIVE", __func__);
        return -1;
    }

    if(NULL == p_phone_num) {
        LV_LOG_ERROR("[%s],p_phone_num is null", __func__);
        return -1;
    }

    /* TODO: when call count > 1, call_idx is useful */

    /* quit current win(incoming/outgoing win),and enter connected win */
    alv_am_win_exit_current(NULL);
    alv_am_win_enter(NULL, CALL_WIN_CONNECT_NAME, p_phone_num);

    return 0;
}

static int32_t win_released_show(uint32_t reason)
{
    if(false == alv_am_app_is_exist(CALL_APP_NAME)) {
        LV_LOG_ERROR("[%s] call app is not exist.", __func__);
        return -1;
    }

    LV_LOG_USER("[%s] reason is %d.", __func__, reason);
    /* TODO: user use release reason to display different tips */

    /* exit incoming/outgoing/connect win */
    if(0 != alv_am_win_exit_current(NULL)) {
        alv_am_app_exit(NULL, "call", NULL);
    }

    return 0;
}

static int32_t ims_status_show(bool ims_on)
{
    return 0;
}

static void call_app_active(lv_obj_t *app, void *p_param)
{
    call_enter_param_t *p_enter_param = NULL;
    if(NULL == app) {
        LV_LOG_ERROR("{%s],app is null",__func__);
        return;
    }
    if(NULL == p_param) {
        LV_LOG_ERROR("{%s],p_param is null,will enter default dialer win ", __func__);
        alv_am_app_set_first_win(app, g_call_dialer_win);
        return;
    } else {
        p_enter_param = (call_enter_param_t *)p_param;
    }

    if(CALL_ORIGIN_GLOBAL_INCOMING == p_enter_param->call_origin) {
        /* first win is incoming win */
        alv_am_app_set_first_win(app, g_call_incoming_win);
        g_call_origin = CALL_ORIGIN_GLOBAL_INCOMING;
    } else if(CALL_ORIGIN_CALLLOG == p_enter_param->call_origin) {
        /* first win is outgoing win */
        alv_am_app_set_first_win(app, g_call_outgoing_win);
        g_call_origin = CALL_ORIGIN_CALLLOG;
    } else if(CALL_ORIGIN_CONTACT == p_enter_param->call_origin) {
        /* first win is outgoing win */
        alv_am_app_set_first_win(app, g_call_outgoing_win);
        g_call_origin = CALL_ORIGIN_CONTACT;
    } else {
        /* first win is dialer win */
        alv_am_app_set_first_win(app, g_call_dialer_win);
        g_call_origin = CALL_ORIGIN_DIALDER;
    }
}

static void call_app_deactive(lv_obj_t *app, void *p_param)
{
    call_status_e current_call_status = call_flow_get_call_status();

    if(NULL == app) {
        LV_LOG_ERROR("[%s],app is null", __func__);
        return;
    }

    /* g_call_origin reset default value */
    g_call_origin = CALL_ORIGIN_DIALDER;

    LV_LOG_INFO("[%s], g_call_origin = %d,call_status = %d", __func__, g_call_origin, current_call_status);
    if(CALL_ORIGIN_GLOBAL_INCOMING == g_call_origin
        && current_call_status == CALL_STATUS_RELEASED) {
        return;
    }

    /* 1. dialer win, incoming, user hangup */
    /* 2. dialer win, incoming ,connected, user hangup */
    /* return */
    if(CALL_ORIGIN_DIALDER == g_call_origin
        && current_call_status == CALL_STATUS_RELEASED) {
        /* do nothing */
        return;
    }

    /* 3. dialer win, quit current app */
    /* 4. global incoming , user hangup */
    /* 5. global incoming , connected, user hangup */
    /* exit call app */
    if(CALL_ORIGIN_GLOBAL_INCOMING == g_call_origin
        && current_call_status == CALL_STATUS_RELEASED) {
        /* do nothing */
    }
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void call_ui_init(void)
{
    /* Create app for call */
    lv_obj_t *app_call = alv_am_app_create(lv_scr_act(), CALL_APP_NAME);
    alv_am_app_set_active(app_call, call_app_active);
    alv_am_app_set_deactive(app_call, call_app_deactive);

    /* dial win */
    g_call_dialer_win = alv_am_win_create(app_call, CALL_WIN_DIAL_NAME);
    alv_am_win_set_active(g_call_dialer_win, dialer_win_active);
    alv_am_win_set_deactive(g_call_dialer_win, dialer_win_deactive);
    alv_am_win_set_focus(g_call_dialer_win, dialer_win_focus);
    alv_am_app_set_first_win(app_call, g_call_dialer_win);

    /* outgoing win */
    g_call_outgoing_win = alv_am_win_create(app_call, CALL_WIN_OUTGOING_NAME);
    alv_am_win_set_active(g_call_outgoing_win, outgoing_win_active);
    alv_am_win_set_deactive(g_call_outgoing_win, outgoing_win_deactive);
    alv_am_win_set_focus(g_call_outgoing_win,outgoing_win_focus);

    /* connected win */
    lv_obj_t *call_connected_win = alv_am_win_create(app_call, CALL_WIN_CONNECT_NAME);
    alv_am_win_set_active(call_connected_win,connected_win_active);
    alv_am_win_set_deactive(call_connected_win, connected_win_deactive);
    alv_am_win_set_focus(call_connected_win, connected_win_focus);

    /* incoming win */
    g_call_incoming_win = alv_am_win_create(app_call, CALL_WIN_INCOMING_NAME);
    alv_am_win_set_active(g_call_incoming_win,incoming_win_active);
    alv_am_win_set_deactive(g_call_incoming_win, incoming_win_deactive);
    alv_am_win_set_focus(g_call_incoming_win, incoming_win_focus);

    /* TODO: 点击图标时进入该app */
    //alv_am_app_enter(NULL, "call");

    call_flow_register_ui_actions(&g_call_wins_flow);
}

