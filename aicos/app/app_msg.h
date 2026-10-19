/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file app_msg.h
 *
 */

#ifndef __APP_MSG_H__
#define __APP_MSG_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

#define __APP_CALLBACK_FUNCTION__(func_name) \
    static int32_t _APP_CALLBACK_##func_name(void *p_param, uint32_t size) \
    {   int32_t ret = 0; \
        ret = app_msg_send_callback(&func_name, p_param, size); \
        if (0 != ret) { \
            app_loge("[%s] app_msg_send_callback error = %d. \n", __func__, ret); \
        } \
        return ret; \
    }

#define __APP_CALLBACK_NAME__(func_name) _APP_CALLBACK_##func_name

/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{
    uint32_t    msg_id;
    void        *p_func_callback;
    void        *p_param;
    uint32_t    size;
}app_callback_msg_t;

//#include "aic_srv_bus.h"
typedef int32_t (*app_msg_func_callback)(void *p_param, uint32_t size); // typedef srv_func_callback

/**********************
 * GLOBAL PROTOTYPES
 **********************/

uint32_t app_msg_send(app_callback_msg_t *p_app_msg, uint32_t wait_option);
uint32_t app_msg_recv(app_callback_msg_t *p_app_msg, uint32_t wait_option);

int app_msg_send_callback(app_msg_func_callback p_func_name, void *p_param, uint32_t size);

int app_msg_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __APP_MSG_H__ */

