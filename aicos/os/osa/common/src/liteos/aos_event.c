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
#define AIC_THIS_FILE    0x003004
#define LOG_MODULE    AMOD_AOS

#include <aic_osa.h>
#include <los_event.h>
#include <los_error.h>
#include <los_interrupt.h>


/**
* aos_create_event() - Create specified event.
* @name_ptr: Specified event name.
*
* Return: Valid event ptr means success, AOS_NULL means error.
*/
aos_event_ptr aos_create_event(char *name_ptr)
{
    EVENT_CB_S *event_ptr = NULL;
    aos_status ret;

    if (!name_ptr)
        return AOS_NULL;

    /* malloc control block for event ptr */
    event_ptr = (EVENT_CB_S *)aos_malloc(sizeof(*event_ptr));
    /* The caller is hard to handler the failed result, assert here. */
    AOS_ASSERT(event_ptr);
    ret = (aos_status)LOS_EventInit(event_ptr);
    if (ret != LOS_OK) {
        aicx_error(0x156493, "sd", "%s: create event ret=%d\n", name_ptr, ret);
        aos_free(event_ptr);
        return AOS_NULL;
    }

    /* event_ptr must be free when deleted */
    return (aos_event_ptr)event_ptr;

}

/**
* aos_delete_event() - Delete specified event.
* @event_ptr: Specified event ptr.
*/
void aos_delete_event(aos_event_ptr event_ptr)
{
    EVENT_CB_S *los_evt_ptr = (EVENT_CB_S *)event_ptr;
    UINT32 ret;

    AOS_ASSERT(los_evt_ptr);

    /* All errors are caused by improper use this api, assert it for debug. */
    ret = LOS_EventDestroy(los_evt_ptr);
    AOS_ASSERT(ret == LOS_OK);

    /* event_ptr is malloc when created, must free it here */
    aos_free(los_evt_ptr);
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
    EVENT_CB_S *los_event_ptr = (EVENT_CB_S *)event_ptr;
    aos_status ret = AOS_SUCCESS;
    u32 mode = 0;
    u32 rflags, before, diff;
    u32 wait_tick = aos_time_to_tick(wait_option);

    AOS_ASSERT(los_event_ptr);

    /* default set no event. */
    *actual_flags_ptr = 0;

    /* check in interrupt routine. */
    if (OS_INT_ACTIVE && (wait_option != AOS_NO_WAIT)) {
        return AOS_WAIT_ERROR;
    }

    /* check and translate get_option. */
    if (get_option > AOS_AND_CLEAR) {
        return AOS_OPTION_ERROR;
    }

    if (get_option < AOS_AND) {
        mode = LOS_WAITMODE_OR;
    } else {
        mode = LOS_WAITMODE_AND;
    }

    if ((get_option == AOS_OR_CLEAR) || (get_option == AOS_AND_CLEAR)) {
        mode |= LOS_WAITMODE_CLR;
    }
    /* disbale los error bit */
    requested_flags = (requested_flags & (~LOS_ERRTYPE_ERROR));

    /* call los interface. */
try_again:
    before = aos_get_tickcount();
    rflags = LOS_EventRead(los_event_ptr, requested_flags,
                            mode, wait_tick);
    switch (rflags) {
    case LOS_ERRNO_EVENT_PTR_NULL:
    case LOS_ERRNO_EVENT_EVENTMASK_INVALID:
    case LOS_ERRNO_EVENT_FLAGS_INVALID:
    case LOS_ERRNO_EVENT_SETBIT_INVALID:
        ret = AOS_OPTION_ERROR;
        break;
    case LOS_ERRNO_EVENT_READ_IN_INTERRUPT:
    case LOS_ERRNO_EVENT_READ_IN_LOCK:
        ret = AOS_WAIT_ERROR;
        break;
    /* @retval 0 The event expected by the user does not occur. */
    case 0:
        diff = aos_get_tickcount() - before;
        if (diff < wait_tick) {
            wait_tick -= diff;
            goto try_again;
        }
        /* Actually timeout, fallthrough! */
    case LOS_ERRNO_EVENT_READ_TIMEOUT:
        ret = AOS_NO_EVENTS;
        break;
    default:
         *actual_flags_ptr = rflags;
        break;
    }
    /* The ret is not success and is not timeout error, print it for debug. */
    if (ret != AOS_SUCCESS && ret != AOS_NO_EVENTS) {
        aicx_error(0x1d5243, "x", "get event rflags=0x%x\n", rflags);
    }

    return ret;
}

/**
* aos_set_event() - Set specified event.
* @event_ptr: Specified event ptr.
* @flags_to_set: Flags to set.
* @set_option: AOS_OR or AOS_AND.
*/
void aos_set_event(aos_event_ptr event_ptr, u32 flags_to_set, u32 set_option)
{
    EVENT_CB_S *los_event_ptr = (EVENT_CB_S *)event_ptr;
    u32 ret;

    AOS_ASSERT(los_event_ptr);
    AOS_ASSERT((set_option == AOS_AND) || (set_option == AOS_OR));

    if (set_option == AOS_AND) {
        ret = LOS_EventClear(los_event_ptr, flags_to_set);
    } else {
        ret = LOS_EventWrite(los_event_ptr, flags_to_set);
    }
    AOS_ASSERT(ret == LOS_OK);
}
