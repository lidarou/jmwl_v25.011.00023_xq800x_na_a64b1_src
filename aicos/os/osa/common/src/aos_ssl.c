/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
*  DESCRIPTION
*
*    This file implement the AIC OSA SSL function for WIN32.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1362c7
#define LOG_MODULE    AMOD_AOS

#include <aic_osa.h>
#include <aos_ssl.h>
#include <aos_config.h>

#define MAX_NAME_LENGTH 0x20

tsl_server_created_func g_tsl_server_created;

/* #ifdef NET_AOS */
#if 1
aos_ssl_info aos_ssl[AOS_MAX_SL_NUMS];
aos_mutex_ptr aos_sl_no_lock;

aos_status aos_init_ssl(void)
{
    u8 sl_index;
    char queue_name[MAX_NAME_LENGTH];
    aos_queue_ptr queue_ptr = AOS_NULL;

    AOS_ASSERT(AOS_MAX_SL_NUMS < 5);

    for (sl_index = 0; sl_index < AOS_MAX_SL_NUMS; sl_index++) {
        /* create queue for Net side */
        /* snprintf(queue_name, MAX_NAME_LENGTH - 1, "net_q_%u", sl_index); */
        queue_ptr = aos_create_queue(queue_name, sizeof(aos_fsm_msg *), 300);
        if (!queue_ptr)
            return AOS_ERROR;
        aos_ssl[sl_index].net_q = queue_ptr;

        /* create queue for UE side */
        /* snprintf(queue_name, MAX_NAME_LENGTH - 1, "ue_q_%u", sl_index); */
        queue_ptr = aos_create_queue(queue_name, sizeof(aos_fsm_msg *), 300);
        if (!queue_ptr) {
            aos_delete_queue(aos_ssl[sl_index].net_q);
            return AOS_ERROR;
        }
        aos_ssl[sl_index].ue_q = queue_ptr;

        aos_ssl[sl_index].port = 0;
        aos_ssl[sl_index].recv_buff = AOS_NULL;
        aos_ssl[sl_index].cur_ptr = AOS_NULL;
        aos_ssl[sl_index].len = 0;
    }

    /* create a lock for aos_sl_no selection */
    aos_sl_no_lock = aos_create_mutex("SSL-LOCK0");
    if (!aos_sl_no_lock) {
        aos_delete_queue(aos_ssl[sl_index].net_q);
        aos_delete_queue(aos_ssl[sl_index].ue_q);
        return AOS_ERROR;
    }

    return AOS_SUCCESS;
}
aos_sl_no aos_create_tsl(u16 port_addr)
{
    u8 index;

    aos_get_mutex(aos_sl_no_lock, AOS_WAIT_FOREVER);

    for (index = 0; index < AOS_MAX_SL_NUMS; index++) {
        if (aos_ssl[index].port == 0) {
            aos_ssl[index].port = port_addr;
            aos_put_mutex(aos_sl_no_lock);
            return index;
        }
    }

    aos_put_mutex(aos_sl_no_lock);

    return AOS_INVALID_SL_NO;
}

aos_sl_no aos_create_tsl_ex(u16 port_addr, aos_sl_addr sl_addr, u8 flag)
{
    u8 index;

    if (flag == AOS_TSL_CLIENT) {
        for (index = 0; index < AOS_MAX_SL_NUMS; index++) {
            if (aos_ssl[index].port == port_addr)
                return index;
        }
        return AOS_INVALID_SL_NO;
    }
    else
        return aos_create_tsl(port_addr);
}

/* net sends msg to ue */
aos_status aos_send_to_tsl_timeout(aos_sl_no sl_no, const u8 *msg_ptr,
                                   u16 len, aos_timeout timeout)
{
    aos_status ret;
    u8 *pucNewMsg = AOS_NULL;

    if (aos_ssl[sl_no].port == 0)
        return AOS_ERROR;

    pucNewMsg = aos_get_mem(AOS_COMMON_POOL_ID, len + 4);
    *(u32 *)pucNewMsg = len;
    aos_memcpy((void *)(pucNewMsg + 4), (const void *)msg_ptr, len);

    ret = aos_send_to_queue(aos_ssl[sl_no].ue_q, (void *)&pucNewMsg, timeout);
    if (ret != AOS_SUCCESS)
          aos_free_mem(pucNewMsg);

    return ret;
}

aos_status aos_send_to_tsl(aos_sl_no sl_no, const u8 *msg_ptr, u16 len)
{
    return aos_send_to_tsl_timeout(sl_no, msg_ptr, len, AOS_WAIT_FOREVER);
}

aos_status _recv_from_tsl_timeout(aos_sl_no sl_no,
                                  aos_queue_ptr queue_ptr,
                                  u8 *msg_ptr,
                                  u16 len,
                                  u16 *actual_len_ptr,
                                  aos_timeout timeout)
{
    aos_status ret;
    u8 *pucNewMsg;
    u32 recv_len = 0;

    AOS_ASSERT(queue_ptr && msg_ptr && actual_len_ptr);

    if (aos_ssl[sl_no].port == 0)
        return AOS_ERROR;

    if (aos_ssl[sl_no].len > 0) {
        if (aos_ssl[sl_no].len > len) {
            aos_memcpy((void *)msg_ptr, (const void *)aos_ssl[sl_no].cur_ptr,
                       len);
            aos_ssl[sl_no].len -= len;
            aos_ssl[sl_no].cur_ptr += len;
            *actual_len_ptr = len;
            return AOS_SUCCESS;
        } else {
            aos_memcpy((void *)msg_ptr, (const void *)aos_ssl[sl_no].cur_ptr,
                       aos_ssl[sl_no].len);
            recv_len += aos_ssl[sl_no].len;
            len -= aos_ssl[sl_no].len;
            msg_ptr += aos_ssl[sl_no].len;
            aos_free_mem(aos_ssl[sl_no].recv_buff);
            aos_ssl[sl_no].recv_buff = AOS_NULL;
            aos_ssl[sl_no].cur_ptr = AOS_NULL;
            aos_ssl[sl_no].len = 0;
        }
    }

    while (len > 0) {
        ret = aos_recv_from_queue(queue_ptr, &pucNewMsg, timeout);
        if (ret == AOS_QUEUE_EMPTY)
            break;

        AOS_ASSERT(pucNewMsg);

        if (*(u32 *)pucNewMsg > len) {
            aos_memcpy((void *)msg_ptr, (const void *)(pucNewMsg + 4), len);
            recv_len += len;
            aos_ssl[sl_no].recv_buff = pucNewMsg;
            aos_ssl[sl_no].cur_ptr = (pucNewMsg + 4 + len);
            aos_ssl[sl_no].len = (*(u32*)pucNewMsg) - len;
            len = 0;
        } else {
            aos_memcpy((void *)msg_ptr, (const void *)(pucNewMsg + 4),
                       *(u32*)pucNewMsg);
            recv_len += *(u32 *)pucNewMsg;
            aos_free_mem(pucNewMsg);
            len -= *(u32 *)pucNewMsg;
            msg_ptr += *(u32 *)pucNewMsg;
        }
        /* break; */    /* no need break here? 2021.12.28 */
    }

    *actual_len_ptr = recv_len;
    return AOS_SUCCESS;
}

aos_status aos_recv_from_tsl_timeout(aos_sl_no sl_no, u8 *msg_ptr, u16 len,
                                     u16 *actual_len_ptr, aos_timeout timeout)
{
    return _recv_from_tsl_timeout(sl_no,
                                  aos_ssl[sl_no].net_q,
                                  msg_ptr,
                                  len,
                                  actual_len_ptr,
                                  timeout);
}

aos_status aos_recv_from_tsl(aos_sl_no sl_no, u8 *msg_ptr, u16 len,
                          u16 *actual_len_ptr)
{
    return _recv_from_tsl_timeout(sl_no,
                                  aos_ssl[sl_no].net_q,
                                  msg_ptr,
                                  len,
                                  actual_len_ptr,
                                  AOS_WAIT_FOREVER);
}

aos_status aos_recv_from_tsl_noblock(aos_sl_no sl_no, u8 *msg_ptr, u16 len,
                                     u16 *actual_len_ptr)
{
    return _recv_from_tsl_timeout(sl_no,
                                  aos_ssl[sl_no].net_q,
                                  msg_ptr,
                                  len,
                                  actual_len_ptr,
                                  AOS_NO_WAIT);
}

aos_status aos_release_all_tsl(void)
{
    u8 sl_index;

    for (sl_index = 0; sl_index < AOS_MAX_SL_NUMS; sl_index++) {
        aos_delete_queue(aos_ssl[sl_index].net_q);
        aos_delete_queue(aos_ssl[sl_index].ue_q);

        aos_ssl[sl_index].net_q = AOS_NULL;
        aos_ssl[sl_index].ue_q = AOS_NULL;
        aos_ssl[sl_index].port = 0;
        aos_ssl[sl_index].recv_buff = AOS_NULL;
        aos_ssl[sl_index].cur_ptr = AOS_NULL;
        aos_ssl[sl_index].len = 0;
    }

    aos_sl_no_lock = AOS_NULL;
    return AOS_SUCCESS;
}

aos_status aos_set_tsl(aos_sl_no sl_no, u16 recv_buf_size, u16 send_buf_size)
{
    return AOS_SUCCESS;
}

aos_status aos_release_tsl(aos_sl_no sl_no)
{
    aos_ssl[sl_no].port = 0;
    aos_ssl[sl_no].recv_buff = AOS_NULL;
    aos_ssl[sl_no].cur_ptr = AOS_NULL;
    aos_ssl[sl_no].len = 0;

    return AOS_SUCCESS;

}

aos_status aos_reset_tsl(aos_sl_no sl_no)
{
    return AOS_SUCCESS;
}

aos_status __declspec(dllexport) net_aos_release_tsl(aos_sl_no sl_no)
{
    return aos_release_tsl(sl_no);
}

__declspec(dllexport) aos_sl_no net_aos_create_tsl(u16 port_addr)
{
    return aos_create_tsl(port_addr);
}

__declspec(dllexport) aos_sl_no net_aos_create_tsl_ex(u16 port_addr,
                                                      aos_sl_addr sl_addr,
                                                      u8 flag)
{
    return aos_create_tsl_ex(port_addr, sl_addr, flag);
}

/* ue sends msg to net */
__declspec(dllexport) aos_status net_aos_send_to_tsl_timeout(aos_sl_no sl_no,
                                                             const u8 *msg_ptr,
                                                             u16 len,
                                                             aos_timeout timeout)
{
    aos_status ret;
    u8 *pucNewMsg = AOS_NULL;

    if (aos_ssl[sl_no].port == 0)
        return AOS_ERROR;

    pucNewMsg = aos_get_mem(AOS_COMMON_POOL_ID, len + 4);
    AOS_ASSERT(pucNewMsg);
    *(u32 *)pucNewMsg = len;
    aos_memcpy((void *)(pucNewMsg + 4), (const void *)msg_ptr, len);

    ret = aos_send_to_queue(aos_ssl[sl_no].net_q, &pucNewMsg, timeout);
    if (ret != AOS_SUCCESS)
          aos_free_mem(pucNewMsg);

    return ret;
}

__declspec(dllexport) aos_status net_aos_send_to_tsl(aos_sl_no sl_no,
                                                     const u8 *msg_ptr,
                                                     u16 len)
{
    return net_aos_send_to_tsl_timeout(sl_no, msg_ptr, len, AOS_WAIT_FOREVER);
}

__declspec(dllexport) aos_status net_aos_recv_from_tsl_timeout(aos_sl_no sl_no,
                                                               u8 *msg_ptr,
                                                               u16 len,
                                                               u16 *actual_len_ptr,
                                                               aos_timeout timeout)
{
    return _recv_from_tsl_timeout(sl_no,
                                  aos_ssl[sl_no].ue_q,
                                  msg_ptr,
                                  len,
                                  actual_len_ptr,
                                  timeout);
}

__declspec(dllexport) aos_status net_aos_recv_from_tsl(aos_sl_no sl_no,
                                                       u8 *msg_ptr, u16 len,
                                                       u16 *actual_len_ptr)
{
    return _recv_from_tsl_timeout(sl_no,
                                  aos_ssl[sl_no].ue_q,
                                  msg_ptr,
                                  len,
                                  actual_len_ptr,
                                  AOS_WAIT_FOREVER);
}

__declspec(dllexport) aos_status net_aos_recv_from_tsl_noblock(aos_sl_no sl_no,
                                                               u8 *msg_ptr,
                                                               u16 len,
                                                               u16 *actual_len_ptr)
{
    return _recv_from_tsl_timeout(sl_no,
                                  aos_ssl[sl_no].ue_q,
                                  msg_ptr,
                                  len,
                                  actual_len_ptr,
                                  AOS_NO_WAIT);
}


#else
//UE
extern __declspec(dllexport) aos_sl_no net_aos_create_tsl(u16);
extern __declspec(dllexport) aos_sl_no net_aos_create_tsl_ex(u16, aos_sl_addr,
                                                             u8);
extern __declspec(dllexport) aos_status net_aos_send_to_tsl(aos_sl_no,
                                                            const u8 *,
                                                            u16);
__declspec(dllexport) aos_status net_aos_send_to_tsl_timeout(aos_sl_no sl_no,
                                                             const u8 *msg_ptr,
                                                             u16 len,
                                                             aos_timeout timeout);
extern __declspec(dllexport) aos_status net_aos_recv_from_tsl(aos_sl_no,
                                                              u8 *,
                                                              u16,
                                                              u16 *);
aos_status __declspec(dllexport) net_aos_release_tsl(aos_sl_no sl_no);
__declspec(dllexport) aos_status net_aos_recv_from_tsl_noblock(aos_sl_no sl_no,
                                                               u8 *msg_ptr,
                                                               u16 len,
                                                               u16 *actual_len_ptr);
__declspec(dllexport) aos_status net_aos_recv_from_tsl_timeout(aos_sl_no sl_no,
                                                               u8 *msg_ptr,
                                                               u16 len,
                                                               u16 *actual_len_ptr,
                                                               aos_timeout timeout);

aos_status aos_init_ssl(void)
{
    return AOS_SUCCESS;
}
aos_sl_no aos_create_tsl(u16 port_addr)
{
    return net_aos_create_tsl(port_addr);
}
aos_sl_no aos_create_tsl_ex(u16 port_addr, aos_sl_addr sl_addr, u8 flag)
{
    return net_aos_create_tsl_ex(port_addr, sl_addr, flag);
}

aos_status aos_send_to_tsl(aos_sl_no sl_no, const u8 *msg_ptr, u16 len)
{
    return net_aos_send_to_tsl(sl_no, msg_ptr, len);
}
aos_status aos_send_to_tsl_timeout(aos_sl_no sl_no, const u8 *msg_ptr, u16 len,
                                   aos_timeout timeout)
{
    return net_aos_send_to_tsl_timeout(sl_no, msg_ptr, len, timeout);
}

aos_status aos_recv_from_tsl(aos_sl_no sl_no, u8 *msg_ptr, u16 len,
                             u16 *actual_len_ptr)
{
    return net_aos_recv_from_tsl(sl_no, msg_ptr, len, actual_len_ptr);
}
aos_status aos_recv_from_tsl_timeout(aos_sl_no sl_no, u8 *msg_ptr, u16 len,
                                     u16 *actual_len_ptr, aos_timeout timeout)
{
    return net_aos_recv_from_tsl_timeout(sl_no, msg_ptr, len,
                                         actual_len_ptr, timeout);
}
aos_status aos_release_all_tsl(void)
{
    return AOS_SUCCESS;
}

aos_status aos_set_tsl(aos_sl_no sl_no, u16 recv_buf_size, u16 send_buf_size)
{
    return AOS_SUCCESS;
}
aos_status aos_release_tsl(aos_sl_no sl_no)
{
    return net_aos_release_tsl(sl_no);

}
aos_status aos_reset_tsl(aos_sl_no sl_no)
{
    return AOS_SUCCESS;
}
aos_status aos_recv_from_tsl_noblock(aos_sl_no sl_no, u8 *msg_ptr, u16 len,
                                     u16 *actual_len_ptr)
{
    return net_aos_recv_from_tsl_noblock(sl_no, msg_ptr, len, actual_len_ptr);
}
#endif

