/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file include APIs for APP msg process.
 */

#include <aic_errno.h>
#include <aic_osa_ex.h>
#include <aos_thread.h>

#include "app_msg.h"

#define APP_CFG_TASK_QUEUE_NUM      (40)
#define APP_CFG_TASK_QUEUE_NAME     "app msg queue"

static aos_queue_ptr app_queue = NULL;

uint32_t app_msg_send(app_callback_msg_t *p_app_msg, uint32_t wait_option)
{
    if(NULL == app_queue) {
        app_loge("[%s] app_queue has NOT initialized !\n", __func__);
        return -1;
    }
    if(NULL == p_app_msg) {
        app_loge("[%s] the pointer of msg is NULL !\n", __func__);
        return -1;
    }
    return aos_send_to_queue(app_queue, (void *)p_app_msg, wait_option);
}

uint32_t app_msg_recv(app_callback_msg_t *p_app_msg, uint32_t wait_option)
{
    if(NULL == app_queue) {
        app_loge("[%s] app_queue has NOT initialized !\n", __func__);
        return -1;
    }
    if(NULL == p_app_msg) {
        app_loge("[%s] the pointer of msg is NULL !\n", __func__);
        return -1;
    }
    return aos_recv_from_queue(app_queue, (void *)p_app_msg, wait_option);
}

int app_msg_send_callback(app_msg_func_callback p_func_name, void *p_param, uint32_t size)
{
    app_callback_msg_t cb_msg = {0};
    uint32_t msg_send_ret = 0;

    if (0 != size) {
        if (NULL == p_param) {
            app_loge("[%s] size is %d ,but p_param is null! \n", __func__, size);
            return -1;
        }
        cb_msg.p_param = aic_malloc(size);
        if(NULL == cb_msg.p_param) {
            app_loge("[%s] cb_msg.p_param malloc fail. \n", __func__);
            return -1;
        }
        memcpy(cb_msg.p_param, p_param, size);
    } else {/* “size == 0” indicate: p_param is data itself, NOT address of pointer */
        cb_msg.p_param = p_param;
    }

    cb_msg.msg_id           = 0xAAAAAAAA;
    cb_msg.p_func_callback  = p_func_name;
    cb_msg.size             = size;

    msg_send_ret = app_msg_send(&cb_msg, AOS_NO_WAIT);
    if (0 != msg_send_ret) {
        app_loge("[%s] msg_send_ret = %d. \n", __func__, msg_send_ret);
        if ((NULL != cb_msg.p_param) && (0 != size)) {
            aic_free(cb_msg.p_param);
        }
        return -1;
    }

    return 0;
}

int app_msg_init(void)
{
    /* Initial app msg queue for extern event handle and dispatch. */
    app_queue = aos_create_queue(   APP_CFG_TASK_QUEUE_NAME,
                                    sizeof(app_callback_msg_t),
                                    APP_CFG_TASK_QUEUE_NUM);
    return 0;
}

