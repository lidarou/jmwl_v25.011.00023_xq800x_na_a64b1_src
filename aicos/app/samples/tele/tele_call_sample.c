/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 * @file tele_call_sample.c
 *
 */
/*********************
 *      INCLUDES
 *********************/
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa_ex.h>
#include <aos_thread.h>
#include "aic_srv_tele.h"

/*********************
 *      DEFINES
 *********************/
#define TELE_CALL_SAMPLE_TASK_STK_SIZE       (4096u)
#define TELE_CALL_SAMPLE_TASK_COM_PRIO       (35u)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/
static int32_t send_dtmf_resp_callback(void *p_param, uint32_t size)
{
    ts_call_send_dtmf_resp_t *p_send_dtmf_resp = NULL;

    if(NULL == p_param) {
        app_loge("[%s]p_param is NULL.\r\n", __func__);
        return -1;
    }

    p_send_dtmf_resp = (ts_call_send_dtmf_resp_t *)p_param;
    app_logi("[%s]ret_value = %d.\r\n",
             __func__,
             p_send_dtmf_resp->ret_value);
    return 0;
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void send_dtmf_sample(void)
{
    int ret = -1;
    ts_call_send_dtmf_info_t send_dtmf_info = {0};
    uint8_t cur_card_id = TS_CARD_ID_MAX;

    /* send single char */
    send_dtmf_info.p_dtmf_str = "1";
    send_dtmf_info.str_length = strlen("1");
    send_dtmf_info.duration = 300;
    aic_srv_tele_sim_get_current_card(&cur_card_id);
    ret = aic_srv_tele_call_send_dtmf(cur_card_id, &send_dtmf_info, send_dtmf_resp_callback);
    app_logi("[%s]send single char ret = %d.", __func__, ret);

    aos_sleep(5000);

    /* send string */
    send_dtmf_info.p_dtmf_str = "1234";
    send_dtmf_info.str_length = strlen("1234");
    send_dtmf_info.duration = 300;
    ret = aic_srv_tele_call_send_dtmf(cur_card_id, &send_dtmf_info, send_dtmf_resp_callback);
    app_logi("[%s]send string ret = %d.", __func__, ret);
}

static void CallSampleEntry(void *thread_input)
{
    while (1) {
        app_logi("Tele Call Sample entry point!\n");

        /* 10 seconds */
        aos_sleep(10000);
        send_dtmf_sample();  /* the sample code for send dtmf */
    }
}

int call_init_sample(void)
{
    aos_create_thread(
        "call_sample_task",
        CallSampleEntry,
        NULL,
        TELE_CALL_SAMPLE_TASK_STK_SIZE,
        0,
        NULL,
        TELE_CALL_SAMPLE_TASK_COM_PRIO,
        AOS_AUTO_START);

    return AIC_SUCCESS;
}

AIC_APP_P2_INIT_EXPORT(call_init_sample);