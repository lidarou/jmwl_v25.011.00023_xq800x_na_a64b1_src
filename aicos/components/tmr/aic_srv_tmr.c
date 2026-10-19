/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file aic_srv_timer.c
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x16ebe0
#define LOG_MODULE    AMOD_SYS


/*********************
 *      INCLUDES
 *********************/
#include <aic_errno.h>
#include <aic_init.h>
#include <aos_thread.h>
#include <aic_osa.h>

#include "aic_srv_tmr.h"

/*********************
 *      DEFINES
 *********************/
#define SRVTIMER_CFG_TASK_STK_SIZE (4096u)
#define SRVTIMER_CFG_TASK_COM_PRIO (30u)


/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    srv_tmr_handle      client_timer_id;        /* user timer id */
    aos_timer_ptr       aos_timer_id;           /* aos timer id */
    srv_func_callback    client_timer_cb;        /* user timer callback */
    bool                is_period;              /* user timer is period */
    void                *p_user_data;           /* user data */
}timer_record_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
#define SRVTIMER_CFG_TASK_QUEUE_NUM      (10)
#define SRVTIMER_CFG_TASK_QUEUE_NAME     "service timer queue"

static aos_queue_ptr srvtimer_queue;
static aos_thread_id  srv_timer_task_id = AOS_INVALID_THREAD_ID;

/**********************
 *  STATIC VARIABLES
 **********************/


/**********************
 *   STATIC FUNCTIONS
 **********************/
/* 1.aos timer callback */
static void aos_timer_timeout_func(void *input)
{
    timer_record_t *p_timer_param = NULL;

    if(input == NULL) {
        aicx_error(0x1ccad8, "s", "[%s],input param is null", __func__);
        return;
    }

    p_timer_param = (timer_record_t *)input;

    //aos_send_to_queue
    if(AOS_SUCCESS != aos_send_to_queue(srvtimer_queue, (void *)p_timer_param, AOS_NO_WAIT)) {
        aicx_error(0x17129d, "s", "[%s],aos_send_msg error", __func__);
    } else {
        aicx_info(0x17f15c, "s", "[%s],aos_send_msg success", __func__);
    }
}

/*1.recieve msg ,find client timer id from timer_record,excute client timer callback,
  2.if timer is not repeat,clear this timer record;else do nothing*/
static void SrvTimerTask(void *thread_input)
{
    timer_record_t timer_record_msg = {0};
    srv_tmr_cb_param_t tmr_cb_param = {0};
    uint32_t data_size = 0;

    uint32_t ret = 0;

    while(1) {
        // recieve msg from queen
        ret = aos_recv_from_queue(srvtimer_queue, (void*)&timer_record_msg, AOS_WAIT_FOREVER);

        if(AOS_SUCCESS != ret) {
            aicx_error(0x1650bf, "s", "[%s],aos_recv_queue error", __func__);
            continue;
        } else {
            /* excute client callback */
             if(NULL != timer_record_msg.client_timer_cb) {
                tmr_cb_param.tmr_handle = timer_record_msg.client_timer_id;
                tmr_cb_param.p_usr_data = timer_record_msg.p_user_data;
                data_size = sizeof(srv_tmr_cb_param_t);
                timer_record_msg.client_timer_cb(&tmr_cb_param, data_size);
            } else {
                aicx_info(0x00e479, "s", "[%s],timer callback is null,do nothing.\r\n", __func__);
            }
            /*TODO:whether destroy this timer,or need user self to destroy it*/
        }
    }
}



/**********************
 *   GLOBAL FUNCTIONS
 **********************/
srv_tmr_handle aic_srv_tmr_create(srv_tmr_param_t *p_tmr_param, srv_func_callback _tmr_cb)
{
    timer_record_t *p_timer_record = NULL;
    aos_timer_ptr   p_aos_timer_id = NULL;

    /* 1. Judge validity of Input param */
    if(NULL == p_tmr_param) {
        aicx_error(0x1a53ec, "s", "[%s],p_tmr_param is null", __func__);
        return NULL;
    }

    /* Judge task has intialed ? */
    if(AOS_INVALID_THREAD_ID == srv_timer_task_id) {
        aicx_error(0x05532f, "s", "[%s],sevice timer task not create", __func__);
        return NULL;
    }

    /* 2. malloc space for record */
    p_timer_record = (timer_record_t *)aos_zalloc(sizeof(timer_record_t));

    if(NULL == p_timer_record) {
        aicx_error(0x095a1a, "s", "[%s],p_timer_record malloc fail", __func__);
        return NULL;
    }

    p_timer_record->p_user_data = p_tmr_param->p_usr_data;
    p_timer_record->client_timer_cb = _tmr_cb;
    p_timer_record->client_timer_id = p_timer_record;

    /* 3. Create timer by AOS API */
    p_aos_timer_id = aos_create_timer( p_tmr_param->p_name,
                                       aos_timer_timeout_func,
                                       (void *)p_timer_record,
                                       p_tmr_param->time_out,
                                       AOS_AUTO_ACTIVATE,
                                       p_tmr_param->is_repeat);

    /* 4. Save aos timer id to timer_record */
    if(NULL != p_aos_timer_id) {
        p_timer_record->aos_timer_id = p_aos_timer_id;
    } else {
        aicx_error(0x0462e4, "s", "[%s],p_aos_timer_id create fail", __func__);
        aos_free(p_timer_record);
        return NULL;
    }

    aicx_info(0x0a06a5, "sx", "[%s],timer create success.timer id = %x", __func__, p_timer_record);
    return (srv_tmr_handle)p_timer_record;
}

int32_t aic_srv_tmr_destroy(srv_tmr_handle tmr_handle)
{
    int32_t ret = 0;
    timer_record_t *timer_info = NULL;

    if(NULL == tmr_handle) {
        aicx_error(0x16caea, "s", "[%s],timer handle is NULL,error", __func__);
        ret = -1;
        return ret;
    }

    timer_info = (timer_record_t *)tmr_handle;

    /* 1. delete aos timer*/
    aos_delete_timer(timer_info->aos_timer_id);

    /* 2. delete srv timer */
    aos_free(timer_info);

    aicx_info(0x14ddce, "s", "[%s],timer destroy success", __func__);

    return ret;
}

int32_t aic_srv_tmr_pause(srv_tmr_handle tmr_handle)
{
    timer_record_t  *p_timer = NULL;
    int32_t  ret = 0;

    if(NULL == tmr_handle) {
        aicx_error(0x16caea, "s", "[%s],timer handle is NULL,error", __func__);
        ret = -1;
        return ret;
    }

    p_timer = (timer_record_t *)tmr_handle;

    /* 1. deactive this timer */
    aos_deactivate_timer(p_timer->aos_timer_id);
    aicx_info(0x190c9d, "s", "[%s],timer pause success.", __func__);

    return ret;
}

int32_t aic_srv_tmr_resume(srv_tmr_handle tmr_handle)
{
    timer_record_t *p_timer = NULL;
    int32_t  ret = 0;

    if(NULL == tmr_handle) {
        aicx_error(0x16caea, "s", "[%s],timer handle is NULL,error", __func__);
        ret = -1;
        return ret;
    }

    p_timer = (timer_record_t *)tmr_handle;

    /* 1. active this timer */
    aos_activate_timer(p_timer->aos_timer_id);
    aicx_info(0x0ca424, "s", "[%s],timer resume success.", __func__);

    return ret;
}

int32_t aic_srv_tmr_change(srv_tmr_handle tmr_handle, srv_tmr_param_t *p_tmr_param)
{
    timer_record_t *p_timer = NULL;
    int32_t ret = 0;

    if(NULL == tmr_handle) {
        aicx_error(0x16caea, "s", "[%s],timer handle is NULL,error", __func__);
        ret = -1;
        return ret;
    }

    if(NULL == p_tmr_param) {
        aicx_error(0x0a1218, "s", "[%s],p_tmr_param NULL,error", __func__);
        ret = -1;
        return ret;
    }

    p_timer = (timer_record_t *)tmr_handle;

    /* 1. change this timer */
    aicx_info(0x1da2cc, "sxdd", "[%s],timer change aos timer id=%x,is_repeat = %d, timeout = %d", __func__, p_timer->aos_timer_id, p_tmr_param->is_repeat,p_tmr_param->time_out);
    aos_change_timer(p_timer->aos_timer_id, p_tmr_param->time_out, p_tmr_param->is_repeat);
    aicx_info(0x142601, "s", "[%s],timer change success.", __func__);

    return ret;
}

int srv_tmr_init(void)
{
    // create msg queen
    srvtimer_queue = aos_create_queue(SRVTIMER_CFG_TASK_QUEUE_NAME,
                                    sizeof(timer_record_t),
                                    SRVTIMER_CFG_TASK_QUEUE_NUM);

    srv_timer_task_id = aos_create_thread(
                                "srv_tmr_task",
                                SrvTimerTask,
                                NULL,
                                SRVTIMER_CFG_TASK_STK_SIZE,
                                0,
                                NULL,
                                SRVTIMER_CFG_TASK_COM_PRIO,
                                AOS_AUTO_START);

    return AIC_SUCCESS;
}

DIS_CHARGE_APP_P1_INIT_EXPORT(srv_tmr_init);

