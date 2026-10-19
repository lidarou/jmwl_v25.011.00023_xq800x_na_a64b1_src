/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log meesage header file.
 */
#ifndef __ALOG_MESSAGE_H__
#define __ALOG_MESSAGE_H__

#include <aipc_api.h>
#include <alog_buffer.h>
#include <alog_msg_format.h>
#include <alog_formater.h>

/**
 * ALOG_MSG- alog message data define.
 *
 * @brr: used to transmit the message log.
 * @buf_mgr: used to manage the messag buffer.
 * @trace_formater: use to format a message buffer in to message frame.
*/
typedef struct ALOG_MSG {
    u32             lost;
    u32             sn;
    ABRR_HANDLE     brr;
    ALFM_T          *msg_formater;
    const           ALMF_OPS_T *ops;
    ALBF_MGR_T      *buf_mgr;
    aos_thread_id   thread;
    aos_queue_ptr   queue;
} ALOG_MSG_T;

ALOG_MSG_T *alog_msg_create(ABRR_HANDLE brr);
void alog_msg_destroy(ALOG_MSG_T *alog_msg);
void almsg_flush_msg_queue_in_assert(void);

#endif /* __ALOG_MESSAGE_H__ */


