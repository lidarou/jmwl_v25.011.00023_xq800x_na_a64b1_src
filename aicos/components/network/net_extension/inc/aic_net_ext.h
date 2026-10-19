/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_NET_EXT_H__
#define __AIC_NET_EXT_H__

#include "nxd_bsd.h"

#ifdef CONFIG_SOCK_CB_MGR
#define AIC_SOCK_CLIENT_QUEUE_NAME "sock client queue"
#define AIC_SOCK_USER_QUEUE_NAME "sock user queue"
#define AIC_SOCK_SEND_QUEUE_NAME "sock send queue"
#define AIC_SOCK_CLIENT_QUEUE_NUM 10
#define AIC_SOCK_USER_QUEUE_NUM 10
#define AIC_SOCK_WAIT_TIME 20
#define AT_SOCK_CONNECT_TIMEOUT (15 * 1000)
#endif

#ifdef CONFIG_SOCK_CB_MGR
typedef int (*sock_callback)(void *p_param, uint32_t size);
typedef struct sock_callback_msg_t
{
    int sock_id;
    u32 msg_id;
    u32 size_to_send;
    u32 size_to_recv;
    u32 size_actual_send;
    u32 size_actual_recv;
    u8 *send_buf;
    u8 *recv_buf;
    u8 *domain;
    u32 port;
    struct sockaddr_in server_addr;
    aos_timer_ptr connecting_timer;
    sock_callback resp_callback;
} SOCK_CB_MSG;

typedef enum {
    SOCK_DNS_PROCESS = 0,
    SOCK_DNS_END,
    SOCK_CONNECTING,
    SOCK_CONNECTED,
    SOCK_SENDING,
    SOCK_SENT,
    SOCK_RECVING,
    SOCK_READ,
    SOCK_RECVED,
    SOCK_DISCONNECTED,
    SOCK_CONNECT_TIMEOUT,
} SOCK_STATUS;
#endif

#ifdef CONFIG_SOCK_CB_MGR
u32 aic_sock_client_msg_send(SOCK_CB_MSG *p_sock_msg, u32 wait_option);
u32 aic_sock_client_msg_recv(SOCK_CB_MSG *p_sock_msg, u32 wait_option);
u32 aic_sock_user_msg_send(SOCK_CB_MSG *p_sock_msg, u32 wait_option);
u32 aic_sock_user_msg_recv(SOCK_CB_MSG *p_sock_msg, u32 wait_option);
u32 aic_sock_user_request(SOCK_CB_MSG *p_sock_req, u32 wait_option,
                          sock_callback resp_callback);
u32 aic_sock_send_data_msg_recv(SOCK_CB_MSG *p_sock_msg, u32 wait_option);
u32 aic_sock_send_data_msg_send(SOCK_CB_MSG *p_sock_msg, u32 wait_option);
#endif

#endif
