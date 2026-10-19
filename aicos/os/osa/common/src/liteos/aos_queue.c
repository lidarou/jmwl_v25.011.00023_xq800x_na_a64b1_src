/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the AIC OSA queue function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0ffbbb
#define LOG_MODULE    AMOD_AOS

#include <aic_osa.h>
#include <aic_assert.h>
#include <aic_math.h>
#include <aic_osa.h>
#include "los_error.h"
#include "los_queue.h"

typedef struct los_queue_stru {
    u32 q_id;
    u8 *queue_start;
    u32 item_size;
    const char *queue_name;
} los_queue_t;

/**
* aos_create_queue() - Create specified queue.
* @queue_name: Specified queue name.
* @item_size: Specified item size, size must be sizeof(ULONG) align,
*             and max value less than 16*sizeof(ULONG).
* @queue_num: Specified queue capacity.
*
* Return: Valid queue ptr means success, AOS_NULL means error.
*/
aos_queue_ptr aos_create_queue(char *queue_name, u32 item_size, u32 queue_num)
{
    u32 ret;
    u32 queue_size;
    u8 *queue_start = AOS_NULL;
    los_queue_t *qcb_ptr = AOS_NULL;

    AIC_ASSERT(queue_name && queue_num && item_size);

    /* check item_size is sizeof(ULONG) align or not */
    AIC_ASSERT(IS_ALIGNED(item_size, sizeof(u32)));

    /* NOTE: Lite queue msgSize = maxMsgSize + sizeof(UINT32); */
    queue_size = (item_size + sizeof(u32)) * queue_num;
    qcb_ptr = (los_queue_t *)aos_malloc(sizeof(los_queue_t));
    queue_start = (u8 *)aos_malloc(queue_size);
    /* The caller is hard to handler the failed result, assert here. */
    AOS_ASSERT(qcb_ptr && queue_start);

    ret = LOS_QueueCreateStatic(queue_name, queue_num,
                                    &qcb_ptr->q_id, queue_start, 0, item_size);
    if (ret != LOS_OK){
        aicx_error(0x0b19ea, "sd", "%s: create mutex ret=%d\n", queue_name, ret);
        aos_free(queue_start);
        aos_free(qcb_ptr);
        return AOS_NULL;
    }
    qcb_ptr->queue_start = queue_start;
    qcb_ptr->item_size = item_size;
    qcb_ptr->queue_name = queue_name;
    return (aos_queue_ptr)qcb_ptr;
}

/**
* aos_delete_queue() - Delete specified queue.
* @queue_ptr: Specified queue ptr
*/
void aos_delete_queue(aos_queue_ptr queue_ptr)
{
    u8 *queue_start;
    u32 ret;
    los_queue_t *qcb_ptr = (los_queue_t *)queue_ptr;

    AIC_ASSERT(qcb_ptr);

    queue_start = (u8 *)qcb_ptr->queue_start;
    AIC_ASSERT(queue_start);

    /* All errors are caused by improper use this api, assert it for debug. */
    ret = LOS_QueueDelete(qcb_ptr->q_id);
    AOS_ASSERT(ret == LOS_OK);

    /* free queue and qcb. */
    aos_free(qcb_ptr);
    aos_free(queue_start);
}

/**
* aos_recv_from_queue() - Get item from specified queue.
* @queue_ptr: Specified queue ptr
* @item_ptr: Specified item ptr to store content from queue.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms)
*
* Return: AOS_SUCCESS means success, others means error.
*/
aos_status aos_recv_from_queue(aos_queue_ptr queue_ptr, void *item_ptr,
                               u32 wait_option)
{
    aos_status ret;
    u32 msg_size;
    los_queue_t *qcb_ptr = (los_queue_t *)queue_ptr;

    AIC_ASSERT(qcb_ptr && item_ptr);

    /* The return status of aos is fully compatibe with threadx. */
    msg_size = qcb_ptr->item_size;
    ret = LOS_QueueReadCopy(qcb_ptr->q_id, (void *)item_ptr, &msg_size, aos_time_to_tick(wait_option));

    /* The ret is not success and is not timeout error, print it for debug. */
    if (ret == LOS_OK) {
        ret = AOS_SUCCESS;
    }
    else if ((ret == LOS_ERRNO_QUEUE_TIMEOUT) || (ret == LOS_ERRNO_QUEUE_ISEMPTY)) {
        ret = AOS_QUEUE_EMPTY;
    }
    else {
        aicx_error(0x01eb2d, "sd", "%s, get queue ret=%d\n", qcb_ptr->queue_name, ret);
        return ret;
    }

    return ret;
}

/**
* aos_send_to_queue() - Send specified item to queue tail.
* @queue_ptr: Specified queue ptr
* @item_ptr: Specified item ptr which points to the content to send
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms)
*
* Return: AOS_SUCCESS means success, others means error.
*/
aos_status aos_send_to_queue(aos_queue_ptr queue_ptr, void *item_ptr,
                             u32 wait_option)
{
    aos_status ret;
    los_queue_t *qcb_ptr = (los_queue_t *)queue_ptr;

    AIC_ASSERT(qcb_ptr && item_ptr);

    /* The return status of aos is not fully compatibe with los. */
    ret = LOS_QueueWriteCopy(qcb_ptr->q_id, (void *)item_ptr,
                                     qcb_ptr->item_size, aos_time_to_tick(wait_option));

    /* The ret is not success and is not timeout error, print it for debug. */
    if (ret != LOS_OK && ret != LOS_ERRNO_QUEUE_ISFULL) {
        aicx_error(0x04c0bb, "sd", "%s, send queue ret=%d\n", qcb_ptr->queue_name, ret);
    }

    if (ret == LOS_ERRNO_QUEUE_ISFULL) {
        ret = AOS_QUEUE_FULL;
    }
    return ret;
}

/**
* aos_send_to_queue() - Send specified item to queue head.
* @queue_ptr: Specified queue ptr
* @item_ptr: Specified item ptr which points to the content to send
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms)
*
* Return: AOS_SUCCESS means success, others means error.
*/
aos_status aos_send_to_queue_head(aos_queue_ptr queue_ptr, void *item_ptr,
                                  u32 wait_option)
{
    aos_status ret;
    los_queue_t *qcb_ptr = (los_queue_t *)queue_ptr;

    AIC_ASSERT(qcb_ptr && item_ptr);

    /* The return status of aos is fully compatibe with threadx. */
    /* The return status of aos is not fully compatibe with los. */
    ret = LOS_QueueWriteHeadCopy(qcb_ptr->q_id, (void *)item_ptr,
                                     qcb_ptr->item_size, aos_time_to_tick(wait_option));

    /* The ret is not success and is not timeout error, print it for debug. */
    if (ret != LOS_OK && ret != LOS_ERRNO_QUEUE_ISFULL) {
        aicx_error(0x04c0bb, "sd", "%s, send queue ret=%d\n", qcb_ptr->queue_name, ret);
    }

    if (ret == LOS_ERRNO_QUEUE_ISFULL) {
        ret = AOS_QUEUE_FULL;
    }
    return ret;

}
