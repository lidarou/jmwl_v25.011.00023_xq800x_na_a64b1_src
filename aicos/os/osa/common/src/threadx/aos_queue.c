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
#define AIC_THIS_FILE    0x0325ef
#define LOG_MODULE    AMOD_AOS

#include <aic_osa.h>
#include <aic_assert.h>
#include <aic_math.h>
#include <aic_osa.h>
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif

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
#ifdef CONFIG_RTOS_THREADX
    UINT ret;
    u32 queue_size;
    u8 *queue_start = AOS_NULL;
    TX_QUEUE *qcb_ptr = AOS_NULL;
#endif

    AIC_ASSERT(queue_name && queue_num && item_size);

#ifdef CONFIG_RTOS_THREADX
    /* check item_size is sizeof(ULONG) align or not */
    AIC_ASSERT(IS_ALIGNED(item_size, sizeof(ULONG)));

    /* malloc queue area and create queue */
    queue_size = item_size * queue_num;
    qcb_ptr = (TX_QUEUE *)aos_malloc(sizeof(TX_QUEUE));
    queue_start = (u8 *)aos_malloc(queue_size);
    /* The caller is hard to handler the failed result, assert here. */
    AOS_ASSERT(qcb_ptr && queue_start);

    ret = tx_queue_create(qcb_ptr, queue_name, item_size / sizeof(ULONG),
                          queue_start, queue_size);
    if (ret != TX_SUCCESS) {
        aicx_error(0x0b19ea, "sd", "%s: create mutex ret=%d\n", queue_name, ret);
        aos_free(queue_start);
        aos_free(qcb_ptr);
        return AOS_NULL;
    }

    return (aos_queue_ptr)qcb_ptr;
#else
    return AOS_NULL;
#endif
}

/**
* aos_delete_queue() - Delete specified queue.
* @queue_ptr: Specified queue ptr
*/
void aos_delete_queue(aos_queue_ptr queue_ptr)
{
#ifdef CONFIG_RTOS_THREADX
    u8 *queue_start;
    UINT ret;
    TX_QUEUE *qcb_ptr = (TX_QUEUE *)queue_ptr;

    AIC_ASSERT(qcb_ptr);

    queue_start = (u8 *)qcb_ptr->tx_queue_start;
    AIC_ASSERT(queue_start);

    /* All errors are caused by improper use this api, assert it for debug. */
    ret = tx_queue_delete(qcb_ptr);
    AOS_ASSERT(ret == AOS_SUCCESS);

    /* free queue and qcb. */
    aos_free(qcb_ptr);
    aos_free(queue_start);
#else
    /* TODO */
#endif
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
#ifdef CONFIG_RTOS_THREADX
    aos_status ret;
    TX_QUEUE *qcb_ptr = (TX_QUEUE *)queue_ptr;

    AIC_ASSERT(qcb_ptr && item_ptr);

    /* The return status of aos is fully compatibe with threadx. */
    ret = tx_queue_receive(qcb_ptr, item_ptr, aos_time_to_tick(wait_option));

    /* The ret is not success and is not timeout error, print it for debug. */
    if (ret != AOS_SUCCESS && ret != AOS_QUEUE_EMPTY)
        aicx_error(0x01eb2d, "sd", "%s, get queue ret=%d\n", qcb_ptr->tx_queue_name, ret);

    return ret;
#else
    return AOS_ERROR;
#endif
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
#ifdef CONFIG_RTOS_THREADX
    aos_status ret;
    TX_QUEUE *qcb_ptr = (TX_QUEUE *)queue_ptr;

    AIC_ASSERT(qcb_ptr && item_ptr);

    /* The return status of aos is fully compatibe with threadx. */
    ret = (aos_status)tx_queue_send(qcb_ptr, item_ptr,
                                    aos_time_to_tick(wait_option));

    /* The ret is not success and is not timeout error, print it for debug. */
    if (ret != AOS_SUCCESS && ret != AOS_QUEUE_FULL)
        aicx_error(0x04c0bb, "sd", "%s, send queue ret=%d\n", qcb_ptr->tx_queue_name, ret);

    return ret;
#else
    return AOS_ERROR;
#endif
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
#ifdef CONFIG_RTOS_THREADX
    aos_status ret;
    TX_QUEUE *qcb_ptr = (TX_QUEUE *)queue_ptr;

    AIC_ASSERT(qcb_ptr && item_ptr);

    /* The return status of aos is fully compatibe with threadx. */
    ret = (aos_status)tx_queue_front_send(qcb_ptr, item_ptr,
                                          aos_time_to_tick(wait_option));

    /* The ret is not success and is not timeout error, print it for debug. */
    if (ret != AOS_SUCCESS && ret != AOS_QUEUE_FULL)
        aicx_error(0x038c0e, "sd", "%s, fsend queue ret=%d\n", qcb_ptr->tx_queue_name, ret);

    return ret;
#else
    return AOS_ERROR;
#endif
}

aos_status aos_get_queue_used_count(aos_queue_ptr queue_ptr, u32 *count)
{
    TX_QUEUE *qcb_ptr = (TX_QUEUE *)queue_ptr;

    AIC_ASSERT(qcb_ptr);
    *count = qcb_ptr->tx_queue_enqueued;

    return AOS_SUCCESS;
}

aos_status aos_get_queue_available_count(aos_queue_ptr queue_ptr, u32 *available_count)
{
    TX_QUEUE *qcb_ptr = (TX_QUEUE *)queue_ptr;

    AIC_ASSERT(qcb_ptr);
    *available_count = qcb_ptr->tx_queue_available_storage;

    return AOS_SUCCESS;
}

boolean aos_get_queue_item_check(aos_queue_ptr queue_ptr, void *item_ptr)
{
    TX_INTERRUPT_SAVE_AREA
    u32 total_num;
    u32 index;
    ULONG *source;
    boolean found = false;

    TX_QUEUE *qcb_ptr = (TX_QUEUE *)queue_ptr;

    AIC_ASSERT(qcb_ptr);

    TX_DISABLE
    /* No. of messages in the queue.  */
    total_num = qcb_ptr->tx_queue_enqueued;

    /* retrieving the message pointer.  */
    source  =  qcb_ptr->tx_queue_read;

    /* check for same item ptr.  */
    for(index = 0; index < total_num; index++)
    {
        if ((u32)item_ptr == (u32)(*source)) {
            found = true;
            break;
        }
        source += qcb_ptr-> tx_queue_message_size;

        if(source == qcb_ptr->tx_queue_end)
            source = qcb_ptr->tx_queue_start;
    }
    TX_RESTORE

    return found;
}

aos_status aos_queue_reset(aos_queue_ptr queue_ptr)
{
    TX_QUEUE *qcb_ptr = (TX_QUEUE *)queue_ptr;

    AIC_ASSERT(qcb_ptr);

    return tx_queue_flush(qcb_ptr);
}
