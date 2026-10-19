#ifndef _ABUP_THREAD_H_
#define _ABUP_THREAD_H_
#include <aic_net_api.h>
#include "nxd_bsd.h"


typedef struct sock_callback_msg_t
{
    int sock_id;
    abup_uint32 msg_id;
    //abup_uint32 size_to_send;
    //abup_uint32 size_to_recv;
    //abup_uint32 size_actual_send;
    //abup_uint32 size_actual_recv;
    //abup_uint8 *send_buf;
    //abup_uint8 *recv_buf;
    //abup_uint8 *domain;
    abup_uint32 port;
    struct sockaddr_in server_addr;
} ABUP_SOCK_CB_MSG;

typedef enum {
    ABUP_SOCK_DNS_PROCESS = 0,
    ABUP_SOCK_DNS_END,
    ABUP_SOCK_CONNECTING,
    ABUP_SOCK_CONNECTED,
    ABUP_SOCK_SENDING,
    ABUP_SOCK_SENT,
    ABUP_SOCK_RECVING,
    ABUP_SOCK_READ,
    ABUP_SOCK_RECVED,
    ABUP_SOCK_DISCONNECTED,
    ABUP_SOCK_CONNECT_TIMEOUT,
    ABUP_SOCK_GETHOSTBYNAME,
    ABUP_SOCK_REG,
    ABUP_SOCK_CV,
    ABUP_SOCK_DL,
    ABUP_SOCK_RU,
    ABUP_SOCK_ME,
} ABUP_SOCK_STATUS;

extern abup_uint32 abup_sock_msg_send(ABUP_SOCK_CB_MSG *p_sock_msg, abup_uint32 wait_option);
extern abup_uint32 abup_sock_msg_recv(ABUP_SOCK_CB_MSG *p_sock_msg, abup_uint32 wait_option);
extern void abup_task_uninit(void);
extern void abup_get_new_version_ex(void);
extern void abup_upgrade_thread_on_ex(void);

#endif
