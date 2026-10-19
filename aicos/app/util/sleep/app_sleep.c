/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file app_sleep.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <lvgl.h>
#include <aic_osa_ex.h>
#include "display.h"
#include "app_sleep.h"
#include "aic_srv_tmr.h"
#include "aic_cmdline.h"
#include "aic_srv_tele.h"


/*********************
 *      DEFINES
 *********************/
#define APP_CFG_MSG_RCV_TIMEOUT         (5)
#define APP_CFG_MSG_RCV_SLP_TIMEOUT     (118000)  /* APP_WDT_TIMEOUT - 2s */

/**********************
 *      TYPEDEFS
 **********************/
#ifdef CONFIG_BL6XX3
extern void ctp_enter_sleep(void);
extern void ctp_exit_sleep(void);
#endif

/**********************
 *  STATIC PROTOTYPES
 **********************/
extern uint32_t g_app_msg_recv_timeout;

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/
#ifdef CONFIG_TP
static int32_t tp_sleep_timer_handle(void *p_param, uint32_t size)
{
    srv_tmr_cb_param_t *p_tmr_cb_param = NULL;
    bool is_sleep = false;

    if(NULL == p_param) {
        return -1;
    }

    p_tmr_cb_param = (srv_tmr_cb_param_t *)p_param;
    is_sleep = (bool)p_tmr_cb_param->p_usr_data;
    app_logi("[%s] is_sleep = %d ",__func__,is_sleep);

    if (false == is_sleep) {
#ifdef CONFIG_BL6XX3
        ctp_exit_sleep();
#endif
    }
    else if (true == is_sleep) {
#ifdef CONFIG_BL6XX3
        ctp_enter_sleep();
#endif
    }
    else {
        app_loge("[%s] is_sleep is err ",__func__);
    }

    if(NULL != p_tmr_cb_param->tmr_handle) {
        aic_srv_tmr_destroy(p_tmr_cb_param->tmr_handle);
    }

    return 0;
}

int enter_tp_sleep(bool is_sleep)
{
    srv_tmr_handle tp_sleep_timer = NULL;
    srv_tmr_param_t tp_sleep_timer_param = {0};
    tp_sleep_timer_param.time_out = 2;/*2ms*/
    tp_sleep_timer_param.is_repeat = 0;
    tp_sleep_timer_param.p_name = "tp sleep timer";
    tp_sleep_timer_param.p_usr_data = (void *)is_sleep;

    tp_sleep_timer = aic_srv_tmr_create(&tp_sleep_timer_param, tp_sleep_timer_handle);
    if (NULL == tp_sleep_timer) {
        app_loge("[%s] create timer fail",__func__);
        return -1;
    }
    return 0;
}
#endif

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int app_enter_sleep(void)
{
    uint8_t current_card_id = 0;
    int32_t ret = 0;

    app_logd("[%s] enter!!!\n", __func__);
    if(aic_is_user_mode()) {
        g_app_msg_recv_timeout = APP_CFG_MSG_RCV_SLP_TIMEOUT;
    } else {
        g_app_msg_recv_timeout = AOS_WAIT_FOREVER;
    }
    display_suspend();
#ifdef CONFIG_TP
    enter_tp_sleep(true);
#endif

    aic_srv_tele_sim_get_current_card(&current_card_id);
    /* only allow report network reg status when app enter sleep, to ensure power consumption */
    ret = aic_srv_tele_radio_set_filter(current_card_id, TS_RADIO_FILTER_REG_STATUS, NULL);
    app_logi("[%s] set_filter NONE ret=%d, current card id=%d\n", __func__, ret, current_card_id);

    return (int)ret;
}

int app_exit_sleep(void)
{
    uint8_t current_card_id = 0;
    int32_t ret = 0;

    app_logd("[%s] enter!!!\n", __func__);
    g_app_msg_recv_timeout = APP_CFG_MSG_RCV_TIMEOUT;
    display_resume();
    lv_obj_invalidate(lv_scr_act());
    lv_timer_handler();
    // send NULL msg to app task

    /*TODO: need optimize */
    aos_sleep(60);
#ifdef CONFIG_TP
    enter_tp_sleep(false);
#endif

    aic_srv_tele_sim_get_current_card(&current_card_id);
    /* allow all status report(include network reg status, signal info, etc.) when app exit sleep */
    ret = aic_srv_tele_radio_set_filter(current_card_id, TS_RADIO_FILTER_ALL, NULL);
    app_logi("[%s] set_filter ALL ret=%d, current card id=%d\n", __func__, ret, current_card_id);

    return (int)ret;
}

