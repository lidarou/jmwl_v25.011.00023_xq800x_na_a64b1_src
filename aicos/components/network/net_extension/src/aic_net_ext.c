/**************************************************************************/
/*                                                                        */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Support Network Extended Functions
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x026d12
#define LOG_MODULE    AMOD_NET

#include <aic_osa.h>
#include <aic_init.h>
#include <aic_errno.h>
#include <aic_size.h>
#include <timer_inc.h>
#include "aic_net_ext.h"
#include "aic_net_api.h"

#ifdef CONFIG_SOCK_CB_MGR
static aos_queue_ptr aic_sock_client_queue;
static aos_queue_ptr aic_sock_user_queue;
static aos_queue_ptr aic_sock_send_queue;
#endif

#ifdef CONFIG_SOCK_CB_MGR

static void sock_connecting_timer_stop(void *input)
{
    SOCK_CB_MSG *sock_msg = (SOCK_CB_MSG *)input;

    if (aos_is_timer_created(sock_msg->connecting_timer)) {
        aos_deactivate_timer(sock_msg->connecting_timer);
        aos_delete_timer(sock_msg->connecting_timer);
        sock_msg->connecting_timer = AIC_NULL;
    } else {
        aicx_info(0x14dbe0, "", "connecting timer has been stopped \n");
    }
}

void sock_connect_timeout(void *input)
{
    SOCK_CB_MSG *sock_msg = (SOCK_CB_MSG *)input;

    sock_msg->msg_id = SOCK_CONNECT_TIMEOUT;
    aic_sock_user_msg_send(sock_msg, AOS_NO_WAIT);
}

static void sock_connecting_timer_start(void *input)
{
    SOCK_CB_MSG *sock_msg = (SOCK_CB_MSG *)input;

    if (!aos_is_timer_created(sock_msg->connecting_timer)) {
        sock_msg->connecting_timer = aos_create_timer("sock_connecting_timer",
                                                      sock_connect_timeout,
                                                      input,
                                                      AT_SOCK_CONNECT_TIMEOUT,
                                                      AOS_AUTO_ACTIVATE,
                                                      AOS_NO_PERIOD);

        AIC_ASSERT(sock_msg->connecting_timer);
    } else {
        aos_change_timer(sock_msg->connecting_timer, AT_SOCK_CONNECT_TIMEOUT,
                         AOS_NO_PERIOD);
    }
}

static void aic_sock_establish_cb(int sock, void* pdata)
{
    SOCK_CB_MSG *sock_msg = (SOCK_CB_MSG *)pdata;

    sock_connecting_timer_stop(pdata);
    sock_msg->msg_id = SOCK_CONNECTED;
    sock_msg->sock_id = sock;

    //aic_sock_client_msg_send(sock_msg, AOS_WAIT_FOREVER);
    if (NULL != sock_msg->resp_callback) {
        sock_msg->resp_callback((void *)sock_msg, sizeof(SOCK_CB_MSG));
    }
    else {
        aicx_error(0x1b5705, "d", "resp_callback is null. msg_id = %d \n",sock_msg->msg_id);
    }
}

static void aic_sock_recv_cb(int sock, void* pdata)
{
    SOCK_CB_MSG *sock_msg = (SOCK_CB_MSG *)pdata;
    sock_msg->msg_id = SOCK_RECVING;
    sock_msg->sock_id = sock;

    //aic_sock_client_msg_send(sock_msg, AOS_WAIT_FOREVER);
    if (NULL != sock_msg->resp_callback) {
        sock_msg->resp_callback((void *)sock_msg, sizeof(SOCK_CB_MSG));
    }
    else {
        aicx_error(0x1b5705, "d", "resp_callback is null. msg_id = %d \n",sock_msg->msg_id);
    }
}

static void aic_sock_disconnect_cb(int sock, void* pdata, int is_active)
{
    SOCK_CB_MSG *sock_msg = (SOCK_CB_MSG *)pdata;
    sock_msg->msg_id = SOCK_DISCONNECTED;
    sock_msg->sock_id = sock;

    //aic_sock_client_msg_send(sock_msg, AOS_WAIT_FOREVER);
    if (NULL != sock_msg->resp_callback) {
        sock_msg->resp_callback((void *)sock_msg, sizeof(SOCK_CB_MSG));
    }
    else {
        aicx_error(0x1b5705, "d", "resp_callback is null. msg_id = %d \n",sock_msg->msg_id);
    }
}

static void aic_sock_client_cb_entry(void *thread_input)
{
    u32 ret = 0;
    SOCK_CB_MSG recv_msg = {0};
    int status;
    struct addrinfo *res;
    struct addrinfo hints;

    aos_memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;

    while (1) {
        ret = aic_sock_client_msg_recv(&recv_msg, AOS_WAIT_FOREVER);
        if( AOS_QUEUE_EMPTY == ret ) {
            continue;
        } else {
            aicx_info(0x012481, "d", "recv msg_id = %d \n",recv_msg.msg_id);
            switch (recv_msg.msg_id) {
            case SOCK_DNS_PROCESS:
                aos_memset(&recv_msg.server_addr, 0, sizeof(struct sockaddr_in));
                status = getaddrinfo(recv_msg.domain, AIC_NULL, &hints, &res);
                if (status == AIC_SUCCESS && res) {
                    if (res->ai_family == AF_INET) {
                        recv_msg.server_addr.sin_family = AF_INET;
                        recv_msg.server_addr.sin_port = htons(recv_msg.port);
                        recv_msg.server_addr.sin_addr.s_addr =
                            ((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr;
                    }

                    freeaddrinfo(res);
                }
                recv_msg.msg_id = SOCK_DNS_END;
                //aic_sock_client_msg_send(&recv_msg, AOS_WAIT_FOREVER);
                if (NULL != recv_msg.resp_callback) {
                    recv_msg.resp_callback((void *)&recv_msg, sizeof(SOCK_CB_MSG));
                }
                else {
                    aicx_error(0x1b5705, "d", "resp_callback is null. msg_id = %d \n",recv_msg.msg_id);
                }
                break;
            case SOCK_CONNECTING:
                sock_connecting_timer_start(&recv_msg);
                aic_socket_register_establish_notify(recv_msg.sock_id,
                                                     aic_sock_establish_cb,
                                                     &recv_msg);

                aic_socket_register_receive_notify(recv_msg.sock_id,
                                                   aic_sock_recv_cb, &recv_msg);

                aic_socket_register_tcp_disconnect_notify(recv_msg.sock_id,
                                                          aic_sock_disconnect_cb,
                                                          &recv_msg);
                break;
            case SOCK_CONNECT_TIMEOUT:
                sock_connecting_timer_stop(&recv_msg);
                recv_msg.msg_id = SOCK_DISCONNECTED;
                //aic_sock_client_msg_send(&recv_msg, AOS_WAIT_FOREVER);
                if (NULL != recv_msg.resp_callback) {
                    recv_msg.resp_callback((void *)&recv_msg, sizeof(SOCK_CB_MSG));
                }
                else {
                    aicx_error(0x1b5705, "d", "resp_callback is null. msg_id = %d \n",recv_msg.msg_id);
                }
                break;
            case SOCK_SENDING:
                aic_sock_send_data_msg_send(&recv_msg, AOS_WAIT_FOREVER);
                break;
            case SOCK_READ:
                #if 0 //Can be implemented by customers.
                recv_msg.size_actual_recv = recv(recv_msg.sock_id,
                                                 (void *)recv_msg.recv_buf,
                                                 recv_msg.size_to_recv, 0);
                #endif

                recv_msg.msg_id = SOCK_RECVED;
                //aic_sock_client_msg_send(&recv_msg, AOS_WAIT_FOREVER);
                if (NULL != recv_msg.resp_callback) {
                    recv_msg.resp_callback((void *)&recv_msg, sizeof(SOCK_CB_MSG));
                }
                else {
                    aicx_error(0x1b5705, "d", "resp_callback is null. msg_id = %d \n",recv_msg.msg_id);
                }
                break;
            default:
                aicx_error(0x195383, "", "Recive a sock msg which is NOT known \n");
                break;
            }
        }
    }
}

static void aic_sock_client_send_entry(void *thread_input)
{
    u32 ret = 0;
    SOCK_CB_MSG recv_msg = {0};
    s32 size_actual_send = 0;
    u32 send_cnt = 0;

     while (1) {
        ret = aic_sock_send_data_msg_recv(&recv_msg, AOS_WAIT_FOREVER);
        if( AOS_QUEUE_EMPTY == ret ) {
            continue;
        } else {
            aicx_info(0x132f47, "d", "recv send msg_id = %d \n",recv_msg.msg_id);
            switch (recv_msg.msg_id) {
            case SOCK_SENDING:
                recv_msg.size_actual_send = 0;
                while(1) {
                    size_actual_send = send(recv_msg.sock_id,
                                            (const char *)recv_msg.send_buf + recv_msg.size_actual_send,
                                            (recv_msg.size_to_send - recv_msg.size_actual_send), 0);

                    if(size_actual_send > 0) {
                        send_cnt = 0;
                        recv_msg.size_actual_send += size_actual_send;
                    } else {
                        if(send_cnt >= 4) {
                            recv_msg.msg_id = SOCK_DISCONNECTED;
                            break;
                        } else {
                            send_cnt++;
                        }
                    }
                    size_actual_send = 0;
                    aicx_info(0x0673e4, "ddd", "[sock] size_actual_send = %d; size_to_send = %d errno = %d\n",
                              recv_msg.size_actual_send, recv_msg.size_to_send, errno);

                    if(recv_msg.size_actual_send == recv_msg.size_to_send) {
                        recv_msg.msg_id = SOCK_SENT;
                        break;
                    }
                    aic_mdelay(50);
                }
                //aic_sock_client_msg_send(&recv_msg, AOS_WAIT_FOREVER);
                if (NULL != recv_msg.resp_callback) {
                    recv_msg.resp_callback((void *)&recv_msg, sizeof(SOCK_CB_MSG));
                } else {
                    aicx_error(0x1b5705, "d", "resp_callback is null. msg_id = %d \n", recv_msg.msg_id);
                }
                break;
            default:
                aicx_error(0x03cbf2, "", "Recive a sock msg which is NOT send data \n");
                break;
            }
        }
    }
}

u32 aic_sock_send_data_msg_recv(SOCK_CB_MSG *p_sock_msg, u32 wait_option)
{
    return aos_recv_from_queue(aic_sock_send_queue, (void *)p_sock_msg,
                               wait_option);
}

u32 aic_sock_send_data_msg_send(SOCK_CB_MSG *p_sock_msg, u32 wait_option)
{
    return aos_send_to_queue(aic_sock_send_queue, (void *)p_sock_msg,
                               wait_option);
}

u32 aic_sock_client_msg_send(SOCK_CB_MSG *p_sock_msg, u32 wait_option)
{
    return aos_send_to_queue(aic_sock_client_queue, (void *)p_sock_msg,
                             wait_option);
}

u32 aic_sock_client_msg_recv(SOCK_CB_MSG *p_sock_msg, u32 wait_option)
{
    return aos_recv_from_queue(aic_sock_user_queue, (void *)p_sock_msg,
                               wait_option);
}

u32 aic_sock_user_msg_send(SOCK_CB_MSG *p_sock_msg, u32 wait_option)
{
    return aos_send_to_queue(aic_sock_user_queue, (void *)p_sock_msg,
                             wait_option);
}

u32 aic_sock_user_msg_recv(SOCK_CB_MSG *p_sock_msg, u32 wait_option)
{
    return aos_recv_from_queue(aic_sock_client_queue, (void *)p_sock_msg,
                               wait_option);
}

u32 aic_sock_user_request(SOCK_CB_MSG *p_sock_req, u32 wait_option, sock_callback resp_callback)
{
    p_sock_req->resp_callback = resp_callback;
    return aos_send_to_queue(aic_sock_user_queue, (void *)p_sock_req,
                             wait_option);
}

int sock_cb_init(void)
{
    aic_sock_client_queue = aos_create_queue(AIC_SOCK_CLIENT_QUEUE_NAME,
                                             sizeof(SOCK_CB_MSG),
                                             AIC_SOCK_CLIENT_QUEUE_NUM);

    AOS_ASSERT(aic_sock_client_queue);

    aic_sock_user_queue = aos_create_queue(AIC_SOCK_USER_QUEUE_NAME,
                                           sizeof(SOCK_CB_MSG),
                                           AIC_SOCK_USER_QUEUE_NUM);

    AOS_ASSERT(aic_sock_client_queue);

    aic_sock_send_queue = aos_create_queue(AIC_SOCK_SEND_QUEUE_NAME,
                                           sizeof(SOCK_CB_MSG),
                                           AIC_SOCK_USER_QUEUE_NUM);

    AOS_ASSERT(aic_sock_send_queue);

    aos_create_thread("aic_sock_client_cb", aic_sock_client_cb_entry, NULL,
                      SIZE_2K, 0, NULL, 30, AOS_AUTO_START);

    aos_create_thread("aic_sock_client_send", aic_sock_client_send_entry, NULL,
                      SIZE_2K, 0, NULL, 30, AOS_AUTO_START);

    return AIC_SUCCESS;
}

DIS_CHARGE_APP_P2_INIT_EXPORT(sock_cb_init);
#endif
