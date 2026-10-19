

#ifdef CONFIG_APP_SMS_FP

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of idle app.
 */
#ifndef __APP_APP_TELE_DATA_FP_MSG_H__
#define __APP_APP_TELE_DATA_FP_MSG_H__

#ifdef __cplusplus
extern "C"
{
#endif


    typedef struct {
        uint32_t        msg_id;
        void *          p_func_callback;
        void *          p_param;
        uint32_t        size;
        } tele_queue_param_t;

#define     TELE_CFG_TASK_QUEUE_NUM (20)

    typedef int32_t(*tele_msg_func_callback_t) (void * p_param,uint32_t param_size); // typedef srv_func_callback


    int tele_data_msg_init(void);
    uint32_t tele_data_msg_recv(tele_queue_param_t * p_app_msg,uint32_t wait_option);
    uint32_t tele_data_msg_send(tele_queue_param_t * p_app_msg,uint32_t wait_option);
    int32_t tele_data_msg_send_callback(tele_msg_func_callback_t p_func_name,void * p_param,uint32_t param_size);


    typedef int32_t(*send_sms_callback_t) (uint32_t send_index,uint32_t send_status,void * user_data);

    typedef struct __SMS_QUEUE_SEND_PARAM_H_ {
        const char *    p_content;
        void * *        p_node;                     //phonebook_map_node_t **p_node;
        uint32_t        node_count;
        send_sms_callback_t fun;
        void *          user_data;
        } sms_queue_send_param_t;

    enum {
        SMS_TASK_QUEUE_SEND = 0x01,
        };

    int32_t tele_data_msg_send_sms(const char * p_content,void * *p_node,uint32_t node_count,send_sms_callback_t fun,
        void * user_data);

#ifdef __cplusplus
} /*extern "C"*/

#endif

#endif /* __APP_APP_TELE_DATA_FP_MSG_H__ */

#endif

