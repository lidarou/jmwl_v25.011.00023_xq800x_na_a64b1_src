/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 * @file tele_sim_sample.c
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
#define TELE_SIM_SAMPLE_TASK_STK_SIZE       (4096u)
#define TELE_SIM_SAMPLE_TASK_COM_PRIO       (35u)

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
static int32_t set_sim_pin_lock_status_resp_callback(void *p_param, uint32_t size)
{
    ts_sim_set_pin_lock_status_resp_t *p_set_pin_lock_status_resp = NULL;

    if(NULL == p_param) {
        app_loge("[%s]p_param is NULL.\r\n", __func__);
        return -1;
    }

    p_set_pin_lock_status_resp = (ts_sim_set_pin_lock_status_resp_t *)(p_param);
    app_logi("[%s]ret_value = %d, remain_times = %d.\r\n",
             __func__,
             p_set_pin_lock_status_resp->ret_value,
             p_set_pin_lock_status_resp->remain_times);
    return 0;
}

static int32_t get_sim_pin_lock_status_resp_callback(void *p_param, uint32_t size)
{
    ts_sim_get_pin_lock_status_resp_t *p_get_pin_lock_status_resp = NULL;

    if(NULL == p_param) {
        app_loge("[%s]p_param is NULL.\r\n", __func__);
        return -1;
    }

    p_get_pin_lock_status_resp = (ts_sim_get_pin_lock_status_resp_t *)p_param;
    app_logi("[%s]ret_value = %d, status = %d.\r\n",
             __func__,
             p_get_pin_lock_status_resp->ret_value,
             p_get_pin_lock_status_resp->status);
    return 0;
}

static int32_t unlock_sim_pin_resp_callback(void *p_param, uint32_t size)
{
    ts_sim_unlock_pin_resp_t *p_unlock_pin_resp = NULL;

    if(NULL == p_param) {
        app_loge("[%s]p_param is NULL.\r\n", __func__);
        return -1;
    }

    p_unlock_pin_resp = (ts_sim_unlock_pin_resp_t *)p_param;
    app_logi("[%s]ret_value = %d, remain_times = %d.\r\n",
             __func__,
             p_unlock_pin_resp->ret_value,
             p_unlock_pin_resp->remain_times);
    return 0;
}

static int32_t change_sim_pin_resp_callback(void *p_param, uint32_t size)
{
    ts_sim_change_pin_resp_t *p_change_pin_resp = NULL;

    if(NULL == p_param) {
        app_loge("[%s]p_param is NULL.\r\n", __func__);
        return -1;
    }

    p_change_pin_resp = (ts_sim_change_pin_resp_t *)p_param;
    app_logi("[%s]ret_value = %d, remain_times = %d.\r\n",
             __func__,
             p_change_pin_resp->ret_value,
             p_change_pin_resp->remain_times);
    return 0;
}

static int32_t get_sim_pin_remain_times_resp_callback(void *p_param, uint32_t size)
{
    ts_sim_get_pin_remain_times_resp_t *p_get_pin_remain_times_resp = NULL;

    if(NULL == p_param) {
        app_loge("[%s]p_param is NULL.\r\n", __func__);
        return -1;
    }

    p_get_pin_remain_times_resp = (ts_sim_get_pin_remain_times_resp_t *)p_param;
    app_logi("[%s]ret_value = %d, remain_times = %d.\r\n",
             __func__,
             p_get_pin_remain_times_resp->ret_value,
             p_get_pin_remain_times_resp->remain_times);
    return 0;
}
/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void set_sim_pin_lock_status_sample(bool is_enable)
{
    int ret = -1;
    ts_sim_pin_lock_status_t lock_status = {0};
    uint8_t cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    lock_status.is_enable = is_enable;
    memcpy(lock_status.pin_pwd, "1234", strlen("1234"));
    ret = aic_srv_tele_sim_set_pin_lock_status(cur_card_id, &lock_status, set_sim_pin_lock_status_resp_callback);
    app_logi("[%s]set true: ret = %d.", __func__, ret);
}

void get_sim_pin_lock_status_sample(void)
{
    int ret = -1;
    uint8_t cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    ret = aic_srv_tele_sim_get_pin_lock_status(cur_card_id, get_sim_pin_lock_status_resp_callback);
    app_logi("[%s]get: ret = %d.", __func__, ret);
}

void unlock_sim_pin_sample(void)
{
    int ret = -1;
    uint8_t cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    ret = aic_srv_tele_sim_unlock_pin(cur_card_id, "1234", unlock_sim_pin_resp_callback);
    app_logi("[%s]get: ret = %d.", __func__, ret);
}

void change_sim_pin_sample(void)
{
    int ret = -1;
    uint8_t cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    ret = aic_srv_tele_sim_change_pin(cur_card_id, "1234", "1234", change_sim_pin_resp_callback);
    app_logi("[%s]get: ret = %d.", __func__, ret);
}

void get_sim_pin_remain_times_sample(void)
{
    int ret = -1;
    uint8_t cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    ret = aic_srv_tele_sim_get_pin_remain_times(cur_card_id, get_sim_pin_remain_times_resp_callback);
    app_logi("[%s]get: ret = %d.", __func__, ret);
}

static void SimSampleEntry(void *thread_input)
{
    while (1) {
        app_logi("Tele Sim Sample entry point!\n");

        /* 10 seconds */
        aos_sleep(15000);
        set_sim_pin_lock_status_sample(true); /* the sample code for set sim pin lock status */

        aos_sleep(5000);
        get_sim_pin_lock_status_sample();  /* the sample code for get sim pin lock status */

        aos_sleep(5000);
        unlock_sim_pin_sample(); /* the sample code for unlock pin */

        aos_sleep(5000);
        change_sim_pin_sample(); /* the sample code for change pin */

        aos_sleep(5000);
        get_sim_pin_remain_times_sample(); /* the sample code for get pin remain times */

        aos_sleep(5000);
        set_sim_pin_lock_status_sample(false);  /* the sample code for set sim pin lock status */

        aos_sleep(5000);
        get_sim_pin_lock_status_sample();  /* the sample code for get sim pin lock status */
    }
}

int sim_init_sample(void)
{
    aos_create_thread(
        "sim_sample_task",
        SimSampleEntry,
        NULL,
        TELE_SIM_SAMPLE_TASK_STK_SIZE,
        0,
        NULL,
        TELE_SIM_SAMPLE_TASK_COM_PRIO,
        AOS_AUTO_START);

    return AIC_SUCCESS;
}

AIC_APP_P2_INIT_EXPORT(sim_init_sample);