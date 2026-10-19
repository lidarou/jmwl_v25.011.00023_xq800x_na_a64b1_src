/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines AOS SSL Interface for WIN32.
 */

#ifndef __AOS_SSL_H__
#define __AOS_SSL_H__

#ifdef WIN32

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
extern "C" {
#endif

#include <aic_osa.h>

/**---------------------------------------------------------------------------*/
/**                          constant variables                               */
/**---------------------------------------------------------------------------*/
#define AOS_MAX_SL_NUMS 4
/*TSL mode:*/
#define AOS_TSL_SERVER  0
#define AOS_TSL_CLIENT  255
#define AOS_UDP_SSL     254

#define AOS_TSL_ASYN_RX 1
#define AOS_TSL_ASYN_TX 2
#define AOS_SSL_ASYN_RX AOS_TSL_ASYN_RX
#define AOS_SSL_ASYN_TX AOS_TSL_ASYN_TX

/*stream flag for TCP socket link*/
#define AOS_TSL_STREAM_FLAG    0xf364
#define AOS_INVALID_SL_NO      201
#define CLOSESOCKET(s)         closesocket(s)

/**---------------------------------------------------------------------------*/
/**                          type define                                      */
/**---------------------------------------------------------------------------*/
typedef u32        aos_timeout;
typedef u8         aos_sl_no;
typedef s32        aos_sock;
typedef u16        aos_port_no;
typedef u8         aos_sl_addr[16]; /*ip address(string)*/
typedef u32        aos_ip_addr;/*ip address Hexadecimal*/

typedef struct aos_ssl_st
{
    u16            port;
    aos_queue_ptr  net_q;    /* net side buffer */
    aos_queue_ptr  ue_q;     /* ue side buffer */

    u8             *recv_buff;
    u8             *cur_ptr;
    u32            len;
} aos_ssl_info;

typedef void (*tsl_server_created_func)(aos_sock sock, u16 port_addr);

/**---------------------------------------------------------------------------*/
/**                          function prototype                               */
/**---------------------------------------------------------------------------*/
extern tsl_server_created_func g_tsl_server_created;

#define aos_asyn_access_ssl(sl_list, sl_nums, flag, timeout) \
    aos_asyn_access_tsl((sl_list), (sl_nums), (flag), (timeout))
#define aos_release_udpsl(sl_no) aos_release_tsl((sl_no))
#define aos_asyn_access_udpsl(sl_list, sl_nums, flag, timeout) \
    aos_asyn_access_tsl((sl_list), (sl_nums), (flag), (timeout))
#define aos_set_udpsl(sl_no, recv_buf_size, send_buf_size) \
    aos_set_tsl((sl_no), (recv_buf_size), (send_buf_size))

/* TCP socket functions */
aos_sl_no aos_create_tsl(u16 port_addr);
/*flag: AOS_TSL_CLIENT, AOS_TSL_SERVER)*/
aos_sl_no aos_create_tsl_ex(u16 port_addr, aos_sl_addr sl_addr, u8 flag);
aos_status aos_set_tsl(aos_sl_no sl_no, u16 recv_buf_size, u16 send_buf_size);
aos_status aos_send_to_tsl(aos_sl_no sl_no, const u8 *msg_ptr, u16 len);
aos_status aos_recv_from_tsl(aos_sl_no sl_no, u8 *msg_ptr, u16 len,
                             u16 *actual_len_ptr);
aos_status aos_reset_tsl(aos_sl_no sl_no);
aos_status aos_release_tsl(aos_sl_no sl_no);
aos_status aos_release_all_tsl(void);
u8 aos_asyn_access_tsl(aos_sl_no *sl_list, u8 sl_nums, u8 flag,
                       aos_timeout timeout);
/* UDP Socket functions */
aos_sl_no aos_create_udpsl(u16 port_addr);
aos_sl_no aos_create_udpsl_ex(u16 local_port,u16 remote_port);
aos_status aos_send_to_udpsl(aos_sl_no sl_no, const u8 *msg_ptr, u16 len);
aos_status aos_recv_from_udpsl(aos_sl_no sl_no, u8 *msg_ptr, u16 len,
                               u16 *actual_len_ptr);
aos_status aos_reset_udpsl(aos_sl_no sl_no);

/* ssl module initialization function */
aos_status aos_init_ssl(void);
aos_status aos_close_socket(aos_sl_no sl_index);

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif/* WIN32 */
#endif/* __AOS_SSL_H__ */

