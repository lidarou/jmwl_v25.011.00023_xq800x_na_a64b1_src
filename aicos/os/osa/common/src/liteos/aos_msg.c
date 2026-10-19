/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the AIC OSA message function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x138758
#define LOG_MODULE    AMOD_AOS

#include <aic_osa.h>
#include <aos_config.h>
#include <aos_thread.h>
#include <aic_assert.h>
#include "los_error.h"
#include "los_queue.h"


extern aos_thread_desc g_thread_desc_tbl[AOS_MAX_THREADS];
extern aos_thread_config g_thread_config_tbl[];
extern u32 get_thread_config_tbl_idx(aos_thread_id thread_id);


PLAT_TCM_CODE static u32 _send_msg(aos_thread_id dest_id, aos_msg_ptr src_msg_ptr,
                                   boolean b_front, boolean b_fsm_msg, u32 wait_option)
{
    u32 ret, tick;
    aos_thread_id thread_id = dest_id;
#ifdef CONFIG_FSMMSG_FEATURE
    aos_fsm_msg *fsm_msg_ptr = AOS_NULL;
#endif
    u32 q_id = INVALID_QUEUE_ID;
    aos_thread_desc *thread_desc;

    AIC_ASSERT(thread_id < AOS_MAX_THREADS);
    AIC_ASSERT(g_thread_desc_tbl[thread_id]);
    thread_desc = g_thread_desc_tbl[thread_id];

#ifdef CONFIG_FSMMSG_FEATURE
    if (b_fsm_msg) {
        fsm_msg_ptr = (aos_fsm_msg *)src_msg_ptr;
        AIC_ASSERT(thread_id == FSM_DEST_THREAD_ID(fsm_msg_ptr));
        if (FSM_SRC_THREAD_ID(fsm_msg_ptr) == FSM_DEST_THREAD_ID(fsm_msg_ptr))
            q_id = thread_desc->intq_id;
        else
            q_id = thread_desc->extq_id;
    } else
#endif

    q_id = thread_desc->extq_id;

    AIC_ASSERT(q_id != INVALID_QUEUE_ID);

    /* translate time(ms) to tick */
    tick = aos_time_to_tick(wait_option);

    if (b_front)
        ret = LOS_QueueWriteHead(q_id, (void *)src_msg_ptr,
                                     sizeof(src_msg_ptr), tick);
    else
        ret = LOS_QueueWrite(q_id, (void *)src_msg_ptr,
                                     sizeof(src_msg_ptr), tick);

    AOS_ASSERT(ret == LOS_OK);
    if (ret != LOS_OK) {
        if (LOS_ERRNO_QUEUE_ISFULL == ret)
            aic_assert_set_queue_full();
        AIC_ASSERT(0);
    }

    return AOS_SUCCESS;
}

/**
* aos_send_msg() - Send specific message to specific thread queue tail.
* @dest_id: Specified thread id.
* @src_msg_ptr: Specified message ptr to send.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms).
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_send_msg(aos_thread_id dest_id, aos_msg_ptr src_msg_ptr,
                 u32 wait_option)
{
    return _send_msg(dest_id, src_msg_ptr, AOS_FALSE, AOS_FALSE, wait_option);
}

/**
* aos_send_msg_head() - Send specific message to specific thread queue head.
* @dest_id: Specified thread id.
* @src_msg_ptr: Specified message ptr to send.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms).
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_send_msg_head(aos_thread_id dest_id, aos_msg_ptr src_msg_ptr,
                      u32 wait_option)
{
    return _send_msg(dest_id, src_msg_ptr, AOS_TRUE, AOS_FALSE, wait_option);
}

static aos_msg_ptr _recv_msg(aos_thread_id dest_id, boolean b_extq,
                            u32 wait_option)
{
    u32 ret, tick;
    u32 msg_size = sizeof(aos_msg_ptr);
    aos_thread_id thread_id = dest_id;
    u32 q_id = INVALID_QUEUE_ID;
    aos_msg_ptr msg_ptr = AOS_NULL;
    aos_thread_desc *thread_desc;

    AIC_ASSERT(thread_id < AOS_MAX_THREADS);
    AIC_ASSERT(g_thread_desc_tbl[thread_id]);
    thread_desc = g_thread_desc_tbl[thread_id];

    if (b_extq)
        q_id = thread_desc->extq_id;
    else
        q_id = thread_desc->intq_id;

    AIC_ASSERT(q_id != INVALID_QUEUE_ID);

    /* translate time(ms) to tick */
    tick = aos_time_to_tick(wait_option);

    ret = LOS_QueueRead(q_id, (void *)&msg_ptr, msg_size, tick);
    AOS_ASSERT(msg_size == sizeof(msg_ptr));
    if (ret == LOS_OK)
        return msg_ptr;
    else if ((ret == LOS_ERRNO_QUEUE_TIMEOUT) || (ret == LOS_ERRNO_QUEUE_ISEMPTY))
        return AOS_NULL;
    else {
        AIC_ASSERT(AOS_FALSE);
        return AOS_NULL;
    }
}

/**
* aos_recv_msg_from_intq() - Receive message from specific thread internal queue
* @dest_id: Specified thread id.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms).
*
* Return: AOS_NULL or valid msg ptr from queue.
*/
aos_msg_ptr aos_recv_msg_from_intq(aos_thread_id dest_id, u32 wait_option)
{
    return _recv_msg(dest_id, AOS_FALSE, wait_option);
}

/**
* aos_recv_msg_from_extq() - Receive message from specific thread external queue
* @dest_id: Specified thread id.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms).
*
* Return: AOS_NULL or valid msg ptr from queue.
*/
aos_msg_ptr aos_recv_msg_from_extq(aos_thread_id dest_id, u32 wait_option)
{
    return _recv_msg(dest_id, AOS_TRUE, wait_option);
}

/**
* aos_recv_msg() - Receive message from specific thread default queue.
* default queue is external queue!
* @dest_id: Specified thread id.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms).
*
* Return: AOS_NULL or valid msg ptr from queue.
*/
aos_msg_ptr aos_recv_msg(aos_thread_id dest_id, u32 wait_option)
{
    return _recv_msg(dest_id, AOS_TRUE, wait_option);
}

#ifdef CONFIG_FSMMSG_FEATURE
/**
* aos_send_fsm_msg_debug() - Send fsm message to specific thread queue tail.
* @dest_id: Specified thread id.
* @src_msg_ptr: Specified fsm message ptr to send.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms).
* @b_trace_msg: TRUE - trace the message.
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_send_fsm_msg_debug(aos_thread_id dest_id, aos_fsm_msg *src_msg_ptr,
                           u32 wait_option, bool b_trace_msg)
{
    /* send TRACE to PC Tool */
    if (b_trace_msg)
        AOS_MESSAGE(src_msg_ptr);

    return _send_msg(dest_id, (aos_msg_ptr)src_msg_ptr, AOS_FALSE, AOS_TRUE,
                     wait_option);
}

/**
* aos_send_fsm_msg_head_debug() - Send fsm message to specific thread queue head.
* @dest_id: Specified thread id.
* @src_msg_ptr: Specified fsm message ptr to send.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms).
* @b_trace_msg: TRUE - trace the message.
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_send_fsm_msg_head_debug(aos_thread_id dest_id, aos_fsm_msg *src_msg_ptr,
                                u32 wait_option, bool b_trace_msg)
{
    /* send TRACE to PC Tool */
    if (b_trace_msg)
        AOS_MESSAGE(src_msg_ptr);

    return _send_msg(dest_id, (aos_msg_ptr)src_msg_ptr, AOS_TRUE, AOS_TRUE,
                     wait_option);
}

/**
* aos_send_fsm_msg_to_api() - Call destination thread's task_main to handle
* specific fsm message directly.
* @dest_id: Specified thread id.
* @src_msg_ptr: Specified fsm message ptr to send.
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
aos_status aos_send_fsm_msg_to_api(aos_fsm_msg *fsm_msg_ptr)
{
    aos_thread_id dest_tid;
    u32 index;

    AIC_ASSERT(fsm_msg_ptr);

    /* send TRACE to PC Tool */
    AOS_MESSAGE(fsm_msg_ptr);

    /* get destination thread id */
    dest_tid = FSM_DEST_THREAD_ID(fsm_msg_ptr);

    /* make sure destination thread has been created */
    AIC_ASSERT(aos_is_thread_created(dest_tid));

    /* call destination thread's task_main */
    index = get_thread_config_tbl_idx(dest_tid);
    (*g_thread_config_tbl[index].task_func.task_main)(fsm_msg_ptr);

    /* free msg */
    /*osa_free_fsm_msg(fsm_msg_ptr);*/

    return AOS_SUCCESS;
}

#ifndef CONFIG_ALOG_MSG_ZERO_COPY
/**
* aos_get_fsm_msg_debug() - Allocate fsm message, include fsm msg header,
* parameter and peer.
* @parameter_len: Specified parameter length.
* @peer_msg_len: Specified peer length.
* @file_name: file name to get fsm msg.
* @line: file line number to get fsm msg.
*
* Return: Valid fsm msg ptr means success, AOS_NULL means error.
*/
aos_fsm_msg *aos_get_fsm_msg_debug(u16 parameter_len,u16 peer_msg_len,
                                   filename_t file_name, u32 line)
{
    aos_fsm_msg* fsm_msg_ptr;

    /* allocate memory for fsm msg header */
    fsm_msg_ptr = (aos_fsm_msg*)aos_get_mem_debug(AOS_FSM_MSG_HEADER_POOL_ID,
                                                  sizeof(aos_fsm_msg),
                                                  AOS_TRUE,
                                                  file_name, line);
    if (!fsm_msg_ptr)
        return AOS_NULL;

    /* allocate memory for parameter */
    if(parameter_len != 0) {
        FSM_PARAM_PTR(fsm_msg_ptr) = aos_get_mem_internel(AOS_COMMON_POOL_ID,
                                                       parameter_len,
                                                       AOS_TRUE,
                                                       file_name,line);
        AOS_ASSERT(FSM_PARAM_PTR(fsm_msg_ptr));
        FSM_PARAM_SIZE(fsm_msg_ptr) = parameter_len;
    }
    else {
        FSM_PARAM_PTR(fsm_msg_ptr) = AOS_NULL;
        FSM_PARAM_SIZE(fsm_msg_ptr) = 0;
    }

    /* rat type no use */
    FSM_DEST_RAT_TYPE(fsm_msg_ptr) = 0xFF;

    /* allocate memory for peer */
    if(peer_msg_len != 0) {
        FSM_PEER_BUF_PTR(fsm_msg_ptr) = aos_get_mem_internel(AOS_COMMON_POOL_ID,
                                                          peer_msg_len,
                                                          AOS_TRUE,
                                                          file_name, line);
        AOS_ASSERT(FSM_PEER_BUF_PTR(fsm_msg_ptr));
        FSM_PEER_BUF_SIZE(fsm_msg_ptr) = peer_msg_len;
        FSM_PEER_BUF_FREE_SIZE(fsm_msg_ptr) = 0;
    }
    else {
        FSM_PEER_BUF_PTR(fsm_msg_ptr) = AOS_NULL;
        FSM_PEER_BUF_SIZE(fsm_msg_ptr) = 0;
        FSM_PEER_BUF_FREE_SIZE(fsm_msg_ptr) = 0;
    }

    FSM_SRC_INSTANCE_ID(fsm_msg_ptr) = 0;
    FSM_DEST_INSTANCE_ID(fsm_msg_ptr) = 0;

    return fsm_msg_ptr;
}

/**
* aos_free_fsm_msg_debug() - Free fsm message, include fsm msg header, parameter
* and peer.
* @fsm_msg_ptr: Specified fsm msg ptr.
* @file_name: file name to free fsm msg.
* @line: file line number to free fsm msg.
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_free_fsm_msg_debug(aos_fsm_msg *fsm_msg_ptr, filename_t file_name, u32 line)
{
    AIC_ASSERT(fsm_msg_ptr != AOS_NULL);

    /* free parameter memory */
    if(FSM_PARAM_PTR(fsm_msg_ptr) != AOS_NULL) {
        aos_free_mem_internel((u8 *)FSM_PARAM_PTR(fsm_msg_ptr), file_name, line);
    }

    /* free peer memory */
    if(FSM_PEER_BUF_PTR(fsm_msg_ptr) != AOS_NULL) {
        aos_free_mem_internel((u8 *)FSM_PEER_BUF_PTR(fsm_msg_ptr),file_name, line);
    }

    /* free fsm msg header */
    aos_free_mem_internel((u8 *)fsm_msg_ptr, file_name, line);

    return AOS_SUCCESS;
}

/**
* aos_link_fsm_msg_debug() - Link fsm message, include fsm msg header, parameter
* and peer.
* @fsm_msg_ptr: Specified fsm msg ptr.
* @file_name: file name to free fsm msg.
* @line: file line number to free fsm msg.
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_link_fsm_msg_debug(aos_fsm_msg *fsm_msg_ptr, filename_t file_name, u32 line)
{
    AIC_ASSERT(fsm_msg_ptr != AOS_NULL);

    /* link parameter memory */
    if(FSM_PARAM_PTR(fsm_msg_ptr) != AOS_NULL)
        aos_link_mem_debug((u8 *)FSM_PARAM_PTR(fsm_msg_ptr), file_name, line);

    /* link peer memory */
    if(FSM_PEER_BUF_PTR(fsm_msg_ptr) != AOS_NULL)
        aos_link_mem_debug((u8 *)FSM_PEER_BUF_PTR(fsm_msg_ptr),file_name, line);

    /* link fsm msg header */
    aos_link_mem_debug((u8 *)fsm_msg_ptr, file_name, line);

    return AOS_SUCCESS;
}
#endif /* CONFIG_ALOG_MSG_ZERO_COPY */
#endif
