/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the AIC OSA timer function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1b9a83
#define LOG_MODULE    AMOD_AOS

#include <aic_assert.h>
#include <aic_osa.h>
#include "los_error.h"
#include "los_interrupt.h"
#include "los_swtmr.h"

typedef struct los_timer_stru {
    u32 timer_id;
    char *name_ptr;
    aos_expiration_func func;
    u32 param;
    void *extension_ptr;
    u32 expiration_time;
    boolean b_period;
} los_timer_t;

static aos_timer_ptr _create_timer(char *name_ptr,
                                   aos_expiration_func expiration_func,
                                   void *input,
                                   u32 expiration_time,
                                   u32 auto_activate,
                                   boolean b_period,
                                   boolean b_fsm_timer)
{
    u32 ret = AOS_SUCCESS;
    los_timer_t *timer_ptr = AOS_NULL;
    u32 param = (u32)input;
    u8 mode;

    AOS_ASSERT(name_ptr && expiration_func);

    /* malloc control block for timer */
    timer_ptr = (los_timer_t *)aos_malloc(sizeof(los_timer_t));
    /* The caller is hard to handler the failed result, assert here. */
    AOS_ASSERT(timer_ptr);

    /* mode set */
    if (b_period) {
        mode = LOS_SWTMR_MODE_PERIOD;
    } else {
        mode = LOS_SWTMR_MODE_NO_SELFDELETE;
    }

    /* save timer ptr to timeout param for fsm timer */
    if (b_fsm_timer)
        param = (u32)timer_ptr;

    timer_ptr->func = expiration_func;
    timer_ptr->param = param;
    timer_ptr->extension_ptr = NULL;
    timer_ptr->name_ptr = name_ptr;
    timer_ptr->expiration_time = expiration_time;
    timer_ptr->b_period = b_period;

    ret = LOS_SwtmrCreate(expiration_time,
                          mode,
                          (SWTMR_PROC_FUNC)expiration_func,
                          &timer_ptr->timer_id,
                          param);

    if (ret != LOS_OK) {
        aicx_error(0x0477ef, "sd", "%s: create timer ret=%d\n", name_ptr, ret);
        aos_free(timer_ptr);
        return AOS_NULL;
    }

    /* auto start */
    if (auto_activate) {
        LOS_SwtmrStart(timer_ptr->timer_id);
    }

    /* timer_ptr must be free when deleted */
    return (aos_timer_ptr)timer_ptr;
}

/**
* aos_create_timer() - Create specified timer.
* @name_ptr: Specified timer name.
* @expiration_func: Specified timer expiration function.
* @input: Expiration function input parameter.
* @expiration_time: Expiration time (ms).
* @auto_activate: AOS_AUTO_ACTIVATE or AOS_NO_ACTIVATE.
* @b_period: AOS_BE_PERIOD or AOS_NO_PERIOD.
*
* Return: Valid timer ptr means success, AOS_NULL means error.
*/
aos_timer_ptr aos_create_timer(char *name_ptr,
                               aos_expiration_func expiration_func, void *input,
                               u32 expiration_time, u32 auto_activate,
                               boolean b_period)
{
    return _create_timer(name_ptr, expiration_func, input,
                         expiration_time, auto_activate, b_period, AOS_FALSE);
}

/**
* aos_delete_timer() - Delete specified timer.
* @timer_ptr: Specified timer ptr.
*/
void aos_delete_timer(aos_timer_ptr timer_ptr)
{
    los_timer_t *los_timer_ptr = (los_timer_t *)timer_ptr;
    u32 ret;

    AOS_ASSERT(los_timer_ptr);

    /* All errors are caused by improper use this api, assert it for debug. */
    (void)LOS_SwtmrStop(los_timer_ptr->timer_id);
    ret = LOS_SwtmrDelete(los_timer_ptr->timer_id);
    AOS_ASSERT(ret == LOS_OK);

    /* timer_ptr is malloc when created, must free it here */
    aos_free(los_timer_ptr);
}

/**
* aos_activate_timer() - Activate specified timer.
* @timer_ptr: Specified timer ptr.
*/
void aos_activate_timer(aos_timer_ptr timer_ptr)
{
    los_timer_t *los_timer_ptr = (los_timer_t *)timer_ptr;
    u32 tick;
    u32 ret;

    AOS_ASSERT(los_timer_ptr);

    /* Just return when already started. */
    ret = LOS_SwtmrTimeGet(los_timer_ptr->timer_id, &tick);
    if (LOS_OK == ret) {
        return;
    }

    ret = LOS_SwtmrStart(los_timer_ptr->timer_id);
    /* All errors are caused by improper use this api, assert it for debug. */
    AOS_ASSERT(ret == LOS_OK);
}

/**
* aos_deactivate_timer() - Deactivate specified timer.
* @timer_ptr: Specified timer ptr.
*/
void aos_deactivate_timer(aos_timer_ptr timer_ptr)
{
    los_timer_t *los_timer_ptr = (los_timer_t *)timer_ptr;

    AOS_ASSERT(los_timer_ptr);

    /* All errors are caused by improper use this api, assert it for debug. */
    (void)LOS_SwtmrStop(los_timer_ptr->timer_id);

}

/**
* aos_change_timer() - Change specified timer's  expiration time.
* @timer_ptr: Specified timer ptr.
* @expiration_time: Expiration time (ms).
* @b_period: AOS_BE_PERIOD or AOS_NO_PERIOD..
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
void aos_change_timer(aos_timer_ptr timer_ptr, u32 expiration_time,
                      boolean b_period)
{
    los_timer_t *los_timer_ptr = (los_timer_t *)timer_ptr;
    u32 ret;
    u8 mode;
    u32 intSave;

    AOS_ASSERT(los_timer_ptr);

    /* mode set */
    if (b_period) {
        mode = LOS_SWTMR_MODE_PERIOD;
    } else {
        mode = LOS_SWTMR_MODE_NO_SELFDELETE;
    }
    /*
     * 1. Before change it, must deactive, otherwise, if the timer is actived,
     * tx_timer_change will do nothing.
     * 2. The 3 apis are always returns success, don't check it.
     */
    intSave = LOS_IntLock();
    ret = LOS_SwtmrDelete(los_timer_ptr->timer_id);
    AOS_ASSERT(ret == LOS_OK);
    ret = LOS_SwtmrCreate(expiration_time,
                          mode,
                          (SWTMR_PROC_FUNC)los_timer_ptr->func,
                          &los_timer_ptr->timer_id,
                          los_timer_ptr->param);
    AOS_ASSERT(ret == LOS_OK);
    ret = LOS_SwtmrStart(los_timer_ptr->timer_id);
    AOS_ASSERT(ret == LOS_OK);
    LOS_IntRestore(intSave);
}

/**
* aos_get_remaining_time() - Get remaining time of specified timer.
* @timer_ptr: Specified timer pointer.
* @value_ptr: Value pointer to save remaining time.
*/
void aos_get_remaining_time(aos_timer_ptr timer_ptr, u32 *value_ptr)
{
    u32 ret;
    u32 tick;
    los_timer_t *los_timer_ptr = (los_timer_t *)timer_ptr;

    AOS_ASSERT(timer_ptr && value_ptr);

    *value_ptr = 0;
    ret = LOS_SwtmrTimeGet(los_timer_ptr->timer_id, &tick);
    if (LOS_OK == ret) {
        *value_ptr = aos_tick_to_time(tick);
        return;
    }
}

/**
* aos_is_timer_created() - Judge specified timer is created or not.
* @timer_ptr: Specified timer pointer.
*
* Return: AOS_TURE means timer created, AOS_FALSE means not created.
*/
boolean aos_is_timer_created(aos_timer_ptr timer_ptr)
{
    los_timer_t *los_timer_ptr = (los_timer_t *)timer_ptr;
    u32 tick;
    u32 ret;

    ret = LOS_SwtmrTimeGet(los_timer_ptr->timer_id, &tick);
    return ((LOS_OK == ret) || (LOS_ERRNO_SWTMR_NOT_STARTED == ret));
}

/**
 * aos_get_timer_info() - Get timer info of specified timer.
 * @timer_ptr: Specified timer pointer.
 * @name: to save the timer name.
 * @active: to save timer active status.
 * @remaining: to save current remaining time(ms).
 * @reschedule: to save reschedule time(ms) fo period timer.
 *
 * Return: AOS_SUCCESS means success , others means error.
 */
aos_status aos_get_timer_info(aos_timer_ptr timer_ptr, char **name, u32 *active,
                              u32 *remaining, u32 *reschedule)
{
    los_timer_t *los_timer_ptr = (los_timer_t *)timer_ptr;
    u32 tick;
    u32 ret;

    AOS_ASSERT(los_timer_ptr);

    *name = los_timer_ptr->name_ptr;
    *active = 0;
    ret = LOS_SwtmrTimeGet(los_timer_ptr->timer_id, &tick);

    if (ret == LOS_OK) {

        *active = 1;

        if (remaining)
            *remaining = aos_tick_to_time((u32)tick);

        if (reschedule)
            *reschedule = los_timer_ptr->expiration_time;
    }

    return ret;
}

#ifdef CONFIG_FSMMSG_FEATURE
#ifdef WIN32
#define AOS_TIMER_THREAD_TID        24
#else
#include <ps_config.h>
#endif

static void _fsm_timer_timeout_func(void *input)
{
    u32 intSave;
    aos_thread_id thread_id;
    aos_fsm_msg *fsm_msg_ptr = AOS_NULL;
    los_timer_t *los_timer_ptr = (los_timer_t *)input;

    AOS_ASSERT(los_timer_ptr);
    intSave = LOS_IntLock();
    fsm_msg_ptr = (aos_fsm_msg *)(los_timer_ptr->extension_ptr);
    if (!fsm_msg_ptr) {
        /* may has been delete by stop or delete timer, just return. */
        LOS_IntRestore(intSave);
        return;
    }

    thread_id = FSM_DEST_THREAD_ID(fsm_msg_ptr);
    FSM_SRC_THREAD_ID(fsm_msg_ptr) = AOS_TIMER_THREAD_TID;
    FSM_SRC_PROCESS_ID(fsm_msg_ptr) = 0;

    /* For period timer: link fsm msg memory to avoid being freed */
    if (los_timer_ptr->b_period) {
        aos_link_fsm_msg(fsm_msg_ptr);
    }
    /* For one shot timer: clear the msg ptr which saved in extension_ptr */
    else {
         los_timer_ptr->extension_ptr = 0;
    }
    LOS_IntRestore(intSave);

    /* send TRACE to PC Tool */
    AOS_MESSAGE(fsm_msg_ptr);

    /* send msg to specified thread, and msg ptr will be freed in that thread */
    if (AOS_SUCCESS != aos_send_msg(thread_id, fsm_msg_ptr, AOS_NO_WAIT))
        aos_free_fsm_msg(fsm_msg_ptr);

}

/**
* aos_create_fsm_timer() - create fsm timer with AOS_NO_ACTIVATE. It will
* provide default timeout value (1000ms) and default timeout func internally.
* @name_ptr: Specified fsm timer name.
* @b_period: AOS_BE_PERIOD or AOS_NO_PERIOD.
*
* Return: Valid timer ptr means success, AOS_NULL means error.
*/
aos_timer_ptr aos_create_fsm_timer(char *name_ptr, boolean b_period)
{
    return _create_timer(name_ptr, _fsm_timer_timeout_func, AOS_NULL,
                         1000, AOS_NO_ACTIVATE, b_period, AOS_TRUE);
}

/**
* aos_start_fsm_timer() - create fsm timer.
* @timer_ptr: Specified fsm timer pointer.
* @expiration_time: Specified fsm timer timeout value.
* @msg_ptr: Specified fsm  msg ptr to send when timeout.
*
* Return: AOS_SUCCESS means success, others means error.
*/
aos_status aos_start_fsm_timer(aos_timer_ptr timer_ptr,
                               u32 expiration_time,
                               aos_fsm_msg* msg_ptr)
{
    u32 intSave;
    los_timer_t *los_timer_ptr = (los_timer_t *)timer_ptr;
    u32 tick;

    AOS_ASSERT(los_timer_ptr);

    /* translate time(ms) to tick */
    tick = aos_time_to_tick(expiration_time);

    (void)LOS_SwtmrStop(los_timer_ptr->timer_id);

    intSave = LOS_IntLock();
    /* update msg ptr */
    los_timer_ptr->extension_ptr = msg_ptr;
    /* change timer */
    aos_change_timer(los_timer_ptr, expiration_time, los_timer_ptr->b_period);
    LOS_IntRestore(intSave);

    return AOS_SUCCESS;
}

/**
* aos_stop_fsm_timer() - Stop fsm timer.
* @timer_ptr: Specified fsm timer pointer.
*/
void aos_stop_fsm_timer(aos_timer_ptr timer_ptr)
{
    u32 intSave;
    los_timer_t *los_timer_ptr = (los_timer_t *)timer_ptr;
    aos_fsm_msg *msg_ptr = AOS_NULL;

    AOS_ASSERT(los_timer_ptr);

    /* deactive timer */
    (void)LOS_SwtmrStop(los_timer_ptr->timer_id);

    intSave = LOS_IntLock();
    /* free fsm msg ptr */
    msg_ptr = (aos_fsm_msg *)(los_timer_ptr->extension_ptr);
    if (!msg_ptr) {
        LOS_IntRestore(intSave);
        return;
    }

    los_timer_ptr->extension_ptr = 0;
    LOS_IntRestore(intSave);

    aos_free_fsm_msg(msg_ptr);

}

/**
* aos_delete_fsm_timer() - Delete specified fsm timer.
* @timer_ptr: Specified fsm timer ptr.
*/
void aos_delete_fsm_timer(aos_timer_ptr timer_ptr)
{
    u32 intSave;
    los_timer_t *los_timer_ptr = (los_timer_t *)timer_ptr;
    aos_fsm_msg *msg_ptr = AOS_NULL;

    AOS_ASSERT(los_timer_ptr);

    intSave = LOS_IntLock();
    /* free fsm msg ptr */
    msg_ptr = (aos_fsm_msg *)(los_timer_ptr->extension_ptr);
    if (msg_ptr) {
        aos_free_fsm_msg(msg_ptr);
        los_timer_ptr->extension_ptr = 0;
    }
    LOS_IntRestore(intSave);

    aos_delete_timer(los_timer_ptr);
}

static void _fsmmsg_timer_timeout_func(void *input)
{
    aos_thread_id thread_id;
    aos_fsmmsgtimer *ptr = input;
    aos_fsm_msg *fsm_msg_ptr = ptr->msg;

    /* has stoped, don't send it. */
    if (ptr->b_stop)
        return;

    thread_id = FSM_DEST_THREAD_ID(fsm_msg_ptr);
    FSM_SRC_THREAD_ID(fsm_msg_ptr) = AOS_TIMER_THREAD_ID;
    FSM_SRC_PROCESS_ID(fsm_msg_ptr) = 0;

    /* send TRACE to PC Tool */
    if (ptr->b_trace)
        AOS_MESSAGE(fsm_msg_ptr);

    /* before send it, link it. */
    aos_link_fsm_msg(fsm_msg_ptr);
    if (AOS_SUCCESS != aos_send_msg(thread_id, fsm_msg_ptr, AOS_NO_WAIT))
        aos_free_fsm_msg(fsm_msg_ptr);
}

/**
* aos_create_fsmmsg_timer() - This function will create a fsmmsg and a timer.
*
* @name_ptr: specified fsm timer name.
* @parameter_len: specified parameter length.
* @peer_msg_len: specified peer length.
* @b_period: AOS_BE_PERIOD or AOS_NO_PERIOD.
* @b_trace: if trace the msg.
*
* Return: Valid timer ptr means success, AOS_NULL means error.
*/
aos_fsmmsgtimer *aos_create_fsmmsg_timer(char *name_ptr,
                                         u16 parameter_len, u16 peer_msg_len,
                                         boolean b_period, boolean b_trace)

{
    aos_fsmmsgtimer *ptr = aos_malloc(sizeof(*ptr));

    ptr->timer = _create_timer(name_ptr, _fsmmsg_timer_timeout_func, ptr,
                               1000, AOS_NO_ACTIVATE, b_period, AOS_FALSE);
    AOS_ASSERT(ptr->timer);

    ptr->msg = aos_get_fsm_msg(parameter_len, peer_msg_len);
    AOS_ASSERT(ptr->msg);

    ptr->b_period = b_period;
    ptr->b_trace = b_trace;
    ptr->b_stop = true;

    return ptr;
}

/**
* aos_start_fsm_timer() - start a msg timer.
* @ptr: the aos_fsmmsgtimer ptr that createed by aos_create_fsmmsg_timer.
* @expiration_time: specified fsmmsg timer timeout value.
*
* Return: AOS_SUCCESS means success, others means error.
*/
aos_status aos_start_fsmmsg_timer(aos_fsmmsgtimer *ptr, u32 expiration_time)
{
    u32 intSave;
    los_timer_t *los_timer_ptr;

    AOS_ASSERT(ptr);
    los_timer_ptr = (los_timer_t *)ptr->timer;

    intSave = LOS_IntLock();
    ptr->b_stop = FALSE;

    /* update timeout value */
    aos_change_timer(los_timer_ptr, expiration_time, los_timer_ptr->b_period);
    LOS_IntRestore(intSave);

    return AOS_SUCCESS;
}


/**
* aos_stop_fsm_timer() - stop a fsmmsg timer.
* @ptr: the aos_fsmmsgtimer ptr that createed by aos_create_fsmmsg_timer.
*/
void aos_stop_fsmmsg_timer(aos_fsmmsgtimer *ptr)
{
    u32 intSave;
    los_timer_t *los_timer_ptr;

    AOS_ASSERT(ptr);
    intSave = LOS_IntLock();
    ptr->b_stop = TRUE;
    LOS_IntRestore(intSave);

    los_timer_ptr = (los_timer_t *)ptr->timer;

    /* deactive timer */
    (void)LOS_SwtmrStop(los_timer_ptr->timer_id);
}

/**
* aos_delete_fsm_timer() - delete a fsmmsg timer.
* Will both delete the timer and the msg.
* @ptr: the aos_fsmmsgtimer ptr that createed by aos_create_fsmmsg_timer.
*/
void aos_delete_fsmmsg_timer(aos_fsmmsgtimer *ptr)
{
    los_timer_t *los_timer_ptr;

    AOS_ASSERT(ptr);

    aos_free_fsm_msg(ptr->msg);

    los_timer_ptr = (los_timer_t *)ptr->timer;
    (void)LOS_SwtmrStop(los_timer_ptr->timer_id);
    (void)LOS_SwtmrDelete(los_timer_ptr->timer_id);

    /* tx_timer_ptr is malloc in aos_create_fsmmsg_timer()->_create_timer(), must free it here */
    aos_free(los_timer_ptr);

    aos_free(ptr);
}
#endif

