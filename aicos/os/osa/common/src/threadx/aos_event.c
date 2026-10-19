/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the AIC OSA event function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0f72c8
#define LOG_MODULE    AMOD_AOS

#include <aic_osa.h>
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif

/**
* aos_create_event() - Create specified event.
* @name_ptr: Specified event name.
*
* Return: Valid event ptr means success, AOS_NULL means error.
*/
aos_event_ptr aos_create_event(char *name_ptr)
{
#ifdef CONFIG_RTOS_THREADX
    TX_EVENT_FLAGS_GROUP *event_ptr = NULL;
    aos_status ret;
#endif

    if (!name_ptr)
        return AOS_NULL;

#ifdef CONFIG_RTOS_THREADX
    /* malloc control block for event ptr */
    event_ptr = (TX_EVENT_FLAGS_GROUP *)aos_malloc(sizeof(*event_ptr));
    /* The caller is hard to handler the failed result, assert here. */
    AOS_ASSERT(event_ptr);
    ret = (aos_status)tx_event_flags_create(event_ptr, name_ptr);
    if (ret != TX_SUCCESS) {
        aicx_error(0x156493, "sd", "%s: create event ret=%d\n", name_ptr, ret);
        aos_free(event_ptr);
        return AOS_NULL;
    }

    /* event_ptr must be free when deleted */
    return (aos_event_ptr)event_ptr;
#else
    return AOS_NULL;
#endif
}

/**
* aos_delete_event() - Delete specified event.
* @event_ptr: Specified event ptr.
*/
void aos_delete_event(aos_event_ptr event_ptr)
{
#ifdef CONFIG_RTOS_THREADX
    TX_EVENT_FLAGS_GROUP *tx_event_ptr = (TX_EVENT_FLAGS_GROUP *)event_ptr;
    UINT ret;

    AOS_ASSERT(tx_event_ptr);

    /* All errors are caused by improper use this api, assert it for debug. */
    ret = tx_event_flags_delete(tx_event_ptr);
    AOS_ASSERT(ret == AOS_SUCCESS);

    /* event_ptr is malloc when created, must free it here */
    aos_free(tx_event_ptr);
#else
    /* TODO */
#endif
}

/**
* aos_get_event() - Get specified event.
* @event_ptr: Specified event ptr.
* @requested_flags: Requested flags.
* @get_option: AOS_OR or AOS_OR_CLEAR or AOS_AND or AOS_AND_CLEAR.
* @actual_flags_ptr: Actual flags of event flag group.
* @wait_option: AOS_NO_WAIT or AOS_WAIT_FOREVER or actual wait time (ms).
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
aos_status aos_get_event(aos_event_ptr event_ptr, u32 requested_flags,
                         u32 get_option, u32 *actual_flags_ptr, u32 wait_option)
{
#ifdef CONFIG_RTOS_THREADX
    TX_EVENT_FLAGS_GROUP *tx_event_ptr = (TX_EVENT_FLAGS_GROUP *)event_ptr;
    aos_status ret = AOS_SUCCESS;

    AOS_ASSERT(tx_event_ptr);

    /* default set no event. */
    *actual_flags_ptr = 0;

    /* The ret is not success and is not timeout error, print it for debug. */
    ret = (aos_status)tx_event_flags_get(tx_event_ptr, (ULONG)requested_flags,
                            (UINT)get_option, (ULONG *)actual_flags_ptr,
                            (ULONG)aos_time_to_tick(wait_option));
    if(ret != TX_SUCCESS && ret != TX_NO_EVENTS)
        aicx_error(0x142b1d, "sd", "%s, get event ret=%d\n",
                   tx_event_ptr->tx_event_flags_group_name, ret);

    return ret;
#else
    return AOS_ERROR;
#endif
}

/**
* aos_set_event() - Set specified event.
* @event_ptr: Specified event ptr.
* @flags_to_set: Flags to set.
* @set_option: AOS_OR or AOS_AND.
*/
void aos_set_event(aos_event_ptr event_ptr, u32 flags_to_set, u32 set_option)
{
#ifdef CONFIG_RTOS_THREADX
    TX_EVENT_FLAGS_GROUP *tx_event_ptr = (TX_EVENT_FLAGS_GROUP *)event_ptr;
    UINT ret;

    AOS_ASSERT(tx_event_ptr);

    ret = tx_event_flags_set(tx_event_ptr, flags_to_set, set_option);
    /* All errors are caused by improper use this api, assert it for debug. */
    AICX_ASSERT_IFX(ret == TX_SUCCESS, 0x1050ef, "d", "set event ret = %d\n", ret);
#else
    /* TODO */
#endif
}
