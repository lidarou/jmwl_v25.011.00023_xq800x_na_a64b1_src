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
#define AIC_THIS_FILE    0x021eee
#define LOG_MODULE    AMOD_AOS

#include <aic_assert.h>
#include <aic_osa.h>
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#include <tx_timer.h>
#endif

#ifdef CONFIG_RTOS_THREADX
typedef VOID (*tx_expiration_function)(ULONG id);
#endif

static aos_timer_ptr _create_timer(char *name_ptr,
                                   aos_expiration_func expiration_func,
                                   void *input,
                                   u32 expiration_time,
                                   u32 auto_activate,
                                   boolean b_period,
                                   boolean b_fsm_timer)
{
    u32 ret = AOS_SUCCESS, tick = 0;
#ifdef CONFIG_RTOS_THREADX
    TX_TIMER *timer_ptr = AOS_NULL;
    ULONG param = (ULONG)input;
#endif

    AOS_ASSERT(name_ptr && expiration_func);

    /* translate time(ms) to tick */
    tick = aos_time_to_tick(expiration_time);

#ifdef CONFIG_RTOS_THREADX
    /* malloc control block for timer */
    timer_ptr = (TX_TIMER *)aos_malloc(sizeof(TX_TIMER));
    /* The caller is hard to handler the failed result, assert here. */
    AOS_ASSERT(timer_ptr);

    /* save timer ptr to timeout param for fsm timer */
    if (b_fsm_timer)
        param = (ULONG)timer_ptr;

    ret = tx_timer_create(timer_ptr, name_ptr,
                          (tx_expiration_function)expiration_func, param,
                          (ULONG)tick,
                          (ULONG)(b_period ? tick : 0),
                          (UINT)auto_activate);

    if (ret != TX_SUCCESS) {
        aicx_error(0x0477ef, "sd", "%s: create timer ret=%d\n", name_ptr, ret);
        aos_free(timer_ptr);
        return AOS_NULL;
    }

    /* timer_ptr must be free when deleted */
    return (aos_timer_ptr)timer_ptr;
#else
    return AOS_NULL;
#endif
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
#ifdef CONFIG_RTOS_THREADX
    TX_TIMER *tx_timer_ptr = (TX_TIMER *)timer_ptr;
    UINT ret;

    AOS_ASSERT(tx_timer_ptr);

    /* All errors are caused by improper use this api, assert it for debug. */
    ret = tx_timer_delete(tx_timer_ptr);
    AOS_ASSERT(ret == AOS_SUCCESS);

    /* timer_ptr is malloc when created, must free it here */
    aos_free(tx_timer_ptr);
#endif
}

/**
* aos_activate_timer() - Activate specified timer.
* @timer_ptr: Specified timer ptr.
*/
void aos_activate_timer(aos_timer_ptr timer_ptr)
{
#ifdef CONFIG_RTOS_THREADX
    TX_TIMER *tx_timer_ptr = (TX_TIMER *)timer_ptr;
    UINT ret;

    AOS_ASSERT(tx_timer_ptr);

    ret = tx_timer_activate(tx_timer_ptr);
    /*
     * The TX_ACTIVATE_ERROR value is
     *  means timer is already active, so can skip it.
     */
    if (TX_ACTIVATE_ERROR == ret)
        return;

    /* All errors are caused by improper use this api, assert it for debug. */
    AOS_ASSERT(ret == AOS_SUCCESS);

#else
    /* TODO */
#endif
}

/**
* aos_deactivate_timer() - Deactivate specified timer.
* @timer_ptr: Specified timer ptr.
*/
void aos_deactivate_timer(aos_timer_ptr timer_ptr)
{
#ifdef CONFIG_RTOS_THREADX
    TX_TIMER *tx_timer_ptr = (TX_TIMER *)timer_ptr;
    UINT ret;

    AOS_ASSERT(tx_timer_ptr);

    /* All errors are caused by improper use this api, assert it for debug. */
    ret = tx_timer_deactivate(tx_timer_ptr);
    AOS_ASSERT(ret == AOS_SUCCESS);

#else
    /* TODO */
#endif
}

/**
* aos_reactive_timer() - Reactive specified timer.
* @timer_ptr: Specified timer ptr.
* @expiration_time: Expiration time (ms).
*
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
void aos_reactive_timer(aos_timer_ptr timer_ptr, u32 expiration_time)
{
#ifdef CONFIG_RTOS_THREADX
    TX_TIMER *tx_timer_ptr = (TX_TIMER *)timer_ptr;
    u32 period, tick = 0;

    AOS_ASSERT(tx_timer_ptr);

    period = tx_timer_ptr->tx_timer_internal.tx_timer_internal_re_initialize_ticks;
    AOS_ASSERT(!period);

    /* translate time(ms) to tick */
    tick = aos_time_to_tick(expiration_time);

    /*
     * 1. Before change it, must deactive, otherwise, if the timer is actived,
     * tx_timer_change will do nothing.
     * 2. The 3 apis are always returns success, don't check it.
     */
    (void)tx_timer_deactivate(tx_timer_ptr);
    (void)tx_timer_change(timer_ptr, (ULONG)tick, 0);
    (void)tx_timer_activate(tx_timer_ptr);
#else
    /* TODO */
#endif
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
#ifdef CONFIG_RTOS_THREADX
    TX_TIMER *tx_timer_ptr = (TX_TIMER *)timer_ptr;
    u32 tick;

    AOS_ASSERT(tx_timer_ptr);

    /* translate time(ms) to tick */
    tick = aos_time_to_tick(expiration_time);

    /*
     * 1. Before change it, must deactive, otherwise, if the timer is actived,
     * tx_timer_change will do nothing.
     * 2. The 3 apis are always returns success, don't check it.
     */
    (void)tx_timer_deactivate(tx_timer_ptr);
    (void)tx_timer_change(timer_ptr, (ULONG)tick,
                          (ULONG)(b_period ? tick : 0));
    (void)tx_timer_activate(tx_timer_ptr);
#else
    /* TODO */
#endif
}

/**
* aos_get_remaining_time() - Get remaining time of specified timer.
* @timer_ptr: Specified timer pointer.
* @value_ptr: Value pointer to save remaining time.
*/
void aos_get_remaining_time(aos_timer_ptr timer_ptr, u32 *value_ptr)
{
#if defined(CONFIG_RTOS_THREADX) && defined(CONFIG_TIMER_TX_NO_TICK_IRQ)
    u32 int_time, sys_time;
    TX_TIMER *tx_timer_ptr = (TX_TIMER *)timer_ptr;

    AOS_ASSERT(timer_ptr && value_ptr);

    int_time = tx_timer_ptr->tx_timer_internal.tx_timer_internal_ticks_onwall;
    sys_time = tx_time_get();
    *value_ptr = (int_time > sys_time) ? (int_time - sys_time) : 0;
#else
    /* TODO */
#endif
}

/**
* aos_get_current_remaining_time() - Get remaining time of current timer.
* @value_ptr: Value pointer to save remaining time.
*/
void aos_get_current_remaining_time(u32 *value_ptr)
{
#if defined(CONFIG_RTOS_THREADX) && defined(CONFIG_TIMER_TX_NO_TICK_IRQ)
    u32 int_time, sys_time;
    TX_TIMER_INTERNAL *tx_timer_ptr = (TX_TIMER_INTERNAL *)_tx_timer_list;

    AOS_ASSERT(value_ptr);

    if (tx_timer_ptr == NULL) {
        *value_ptr = AOS_WAIT_FOREVER;
        return;
    }
    int_time = tx_timer_ptr->tx_timer_internal_ticks_onwall;
    sys_time = tx_time_get();
    *value_ptr = (int_time > sys_time) ? (int_time - sys_time) : 0;
#else
    /* TODO */
#endif
}

/**
* aos_is_timer_created() - Judge specified timer is created or not.
* @timer_ptr: Specified timer pointer.
*
* Return: AOS_TURE means timer created, AOS_FALSE means not created.
*/
boolean aos_is_timer_created(aos_timer_ptr timer_ptr)
{
#ifdef CONFIG_RTOS_THREADX
    TX_TIMER *tx_timer_ptr = (TX_TIMER *)timer_ptr;

    return (tx_timer_ptr && tx_timer_ptr->tx_timer_id == TX_TIMER_ID);

#else
    return AOS_FALSE;
#endif
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
#ifdef CONFIG_RTOS_THREADX
    aos_status ret;
    u32 remaining_ticks, reschedule_ticks;
    TX_TIMER *tx_timer_ptr = (TX_TIMER *)timer_ptr;

    ret = tx_timer_info_get(tx_timer_ptr, name, active,
                            (ULONG *)&remaining_ticks,
                            (ULONG *)&reschedule_ticks,
                            AOS_NULL);

    if (ret == AOS_SUCCESS) {
        if (remaining)
            *remaining = aos_tick_to_time((u32)remaining_ticks);

        if (reschedule)
            *reschedule = aos_tick_to_time((u32)reschedule);
    }

    return ret;
#else
    return AOS_ERROR;
#endif
}


/**
 * The following api is used for relaxed functions.
 */

AOS_RELAXED_TMR *g_tmr_relaxed_list;

void aos_relaxed_timer_callback(void *param)
{
    AOS_RELAXED_TMR *cur_timer = (AOS_RELAXED_TMR *)param;

    AOS_ASSERT(cur_timer);

    /* Call the timer-expiration function, if non-NULL. */
    if (cur_timer->func != TX_NULL)
    {
        (cur_timer->func)(cur_timer->input);
    }

    /* Deactive this timer, remove it from g_tmr_relaxed_list */
    aos_deactivate_relaxed_timer(cur_timer);

    if (cur_timer->b_period == AOS_BE_PERIOD) {
        (void)tx_timer_change(cur_timer->relaxed_timer, (ULONG)cur_timer->relaxed_time, 0);
        aos_activate_relaxed_timer(cur_timer);
    }
}

void aos_strict_timer_callback(void *param)
{
    AOS_RELAXED_TMR *cur_timer = (AOS_RELAXED_TMR *)param;

    AOS_ASSERT(cur_timer);

    /* Call the timer-expiration function, if non-NULL. */
    if (cur_timer->func != TX_NULL)
    {
        (cur_timer->func)(cur_timer->input);
    }

    /* Deactive this timer, remove it from g_tmr_relaxed_list */
    aos_deactivate_relaxed_timer(cur_timer);

    if (cur_timer->b_period == AOS_BE_PERIOD) {
        (void)tx_timer_change(cur_timer->relaxed_timer, (ULONG)cur_timer->relaxed_time, 0);
        (void)tx_timer_change(cur_timer->strict_timer, (ULONG)cur_timer->strict_time, 0);
        aos_activate_relaxed_timer(cur_timer);
        cur_timer->status = RLX_TMR_ACTIVE;
    }
}

/**
 * aos_create_relaxed_timer() - Create relaxed timer.
 * @name_ptr: Relaxed timer name.
 * @expiration_func: Specified timer expiration function.
 * @input: Expiration function input parameter.
 * @relaxed_time: Expiration relaxed time (ms), which do not wakeup system.
 * @strict_time: Expiration strict time (ms), which will wakeup system.
 * @auto_activate: AOS_AUTO_ACTIVATE or AOS_NO_ACTIVATE.
 * @b_period: AOS_BE_PERIOD or AOS_NO_PERIOD.
 *
 * Return: Valid timer ptr means success, AOS_NULL means error.
 */
aos_rlx_timer_ptr aos_create_relaxed_timer(char *name_ptr,
                            aos_expiration_func expiration_func, void *input,
                            u32 relaxed_time, u32 strict_time, u32 auto_activate,
                            boolean b_period)
{
    aos_rlx_timer_ptr tm_ptr, str_tmr_ptr = NULL;
    AOS_RELAXED_TMR *rel_tmr;

    AIC_ASSERT(relaxed_time && strict_time && relaxed_time < strict_time);

    /* malloc control block for relaxed timer */
    rel_tmr = (AOS_RELAXED_TMR *)aos_zalloc(sizeof(AOS_RELAXED_TMR));
    AOS_ASSERT(rel_tmr);

    tm_ptr = aos_create_timer(name_ptr, aos_relaxed_timer_callback, rel_tmr,
                            relaxed_time, AOS_NO_ACTIVATE, AOS_NO_PERIOD);
    AIC_ASSERT(tm_ptr);

    if (strict_time != AOS_WAIT_FOREVER) {
        str_tmr_ptr = aos_create_timer(name_ptr, aos_strict_timer_callback, rel_tmr,
                                strict_time, AOS_NO_ACTIVATE, AOS_NO_PERIOD);
        AIC_ASSERT(str_tmr_ptr);
    }

    rel_tmr->relaxed_time   = relaxed_time;
    rel_tmr->strict_time    = strict_time;
    rel_tmr->func           = expiration_func;
    rel_tmr->input          = input;
    rel_tmr->relaxed_timer  = tm_ptr;
    rel_tmr->strict_timer   = str_tmr_ptr;
    rel_tmr->b_period       = b_period;
    rel_tmr->status         = RLX_TMR_IDLE;

    if (auto_activate == AOS_AUTO_ACTIVATE)
        aos_activate_relaxed_timer(rel_tmr);

    return (aos_rlx_timer_ptr)rel_tmr;
}

/**
 * aos_delete_relaxed_timer() - Delete relaxed timer.
 * @timer_ptr: Relaxed timer ptr.
 */
void aos_delete_relaxed_timer(aos_rlx_timer_ptr timer_ptr)
{
    AOS_RELAXED_TMR *rel_tmr = (AOS_RELAXED_TMR *)timer_ptr;

    AOS_ASSERT(rel_tmr);

    if (rel_tmr->status < RLX_TMR_DEACTIVE)
        aos_deactivate_relaxed_timer(rel_tmr);

    aos_delete_timer(rel_tmr->relaxed_timer);
    if (rel_tmr->strict_timer != NULL)
        aos_delete_timer(rel_tmr->strict_timer);
    rel_tmr->status = RLX_TMR_DELETE;

    /* timer_ptr is malloc when created, must free it here */
    aos_free(rel_tmr);
}

/**
 * aos_activate_relaxed_timer() - Activate relaxed timer.
 * @timer_ptr: Relaxed timer ptr.
 */
void aos_activate_relaxed_timer(aos_rlx_timer_ptr timer_ptr)
{
    AOS_RELAXED_TMR *rel_tmr = (AOS_RELAXED_TMR *)timer_ptr;
    AOS_RELAXED_TMR **timer_list, *next_timer;

    TX_INTERRUPT_SAVE_AREA

    AOS_ASSERT(rel_tmr);

    TX_DISABLE;
    /* Active relaxed timer. */
    aos_activate_timer(rel_tmr->relaxed_timer);

    /* Calculate the proper place for the timer. */
    timer_list = &g_tmr_relaxed_list;
    if ((*timer_list) == TX_NULL) {
        /* This list is NULL, just put the new timer on it. */

        /* Setup the links in this timer. */
        rel_tmr->rel_next     = rel_tmr;
        rel_tmr->rel_previous = rel_tmr;
        *timer_list = rel_tmr;
    } else {
        /*
         * This list is not NULL, add current timer to the sort list.
         * Loop check if the timer is in the list already.
         */
        for (next_timer = (*timer_list);
             next_timer != rel_tmr;
             next_timer = next_timer->rel_next) {
            /* This is the last timer, insert into tail of the list */
            if (next_timer->rel_next == (*timer_list)) {
                next_timer->rel_next        = rel_tmr;
                (*timer_list)->rel_previous = rel_tmr;
                rel_tmr->rel_next           = (*timer_list);
                rel_tmr->rel_previous       = next_timer;
                break;
            }
        }
    }
    rel_tmr->status = RLX_TMR_ACTIVE;
    TX_RESTORE;
}

/**
 * aos_deactivate_relaxed_timer() - Deactivate relaxed timer.
 * @timer_ptr: Relaxed timer ptr.
 */
void aos_deactivate_relaxed_timer(aos_rlx_timer_ptr timer_ptr)
{
    AOS_RELAXED_TMR *rel_tmr = (AOS_RELAXED_TMR *)timer_ptr;
    AOS_RELAXED_TMR **timer_list, *next_timer, *previous_timer;

    TX_INTERRUPT_SAVE_AREA

    AOS_ASSERT(rel_tmr);

    timer_list = &g_tmr_relaxed_list;
    if ((*timer_list) == NULL)
        return ;

    TX_DISABLE;

    /* it is the first timer needs to be deleted */
    if ((*timer_list) == rel_tmr) {
        /* there is only one timer in the list */
        if ((*timer_list) == (*timer_list)->rel_next) {
            *timer_list = NULL;
        } else {
            (*timer_list) = (*timer_list)->rel_next;
        }
        goto relaxed_del;
    }

    for (next_timer = (*timer_list)->rel_next;
         next_timer != (*timer_list);
         next_timer = next_timer->rel_next) {
        /* Find the timer in the list and remove it. */
        if (next_timer == rel_tmr) {
            previous_timer           = rel_tmr->rel_previous;
            next_timer               = rel_tmr->rel_next;
            previous_timer->rel_next = next_timer;
            next_timer->rel_previous = previous_timer;
            break;
        }
    }

relaxed_del:

    aos_deactivate_timer(rel_tmr->relaxed_timer);

    rel_tmr->status = RLX_TMR_DEACTIVE;

    TX_RESTORE;
}

/**
 * aos_reactive_relaxed_timer() - Reactive relaxed timer.
 * @timer_ptr: Relaxed timer ptr.
 *
 * Return: AOS_SUCCESS means success, AOS_ERROR means error.
 */
void aos_reactive_relaxed_timer(aos_rlx_timer_ptr timer_ptr)
{
    AOS_RELAXED_TMR *rel_tmr = (AOS_RELAXED_TMR *)timer_ptr;

    AOS_ASSERT(rel_tmr && rel_tmr->b_period == AOS_NO_PERIOD);

    aos_deactivate_relaxed_timer(timer_ptr);

    (void)tx_timer_change(rel_tmr->relaxed_timer, rel_tmr->relaxed_time, 0);

    aos_activate_relaxed_timer(timer_ptr);
}

/**
 * aos_change_relaxed_timer() - Change relaxed timer's  expiration time.
 * @timer_ptr: Relaxed timer ptr.
 * @expiration_time: Expiration time (ms).
 * @b_period: AOS_BE_PERIOD or AOS_NO_PERIOD..
 *
 * Return: AOS_SUCCESS means success, AOS_ERROR means error.
 */
void aos_change_relaxed_timer(aos_rlx_timer_ptr timer_ptr, u32 relaxed_time, u32 strict_time,
                            boolean b_period)
{
    AOS_RELAXED_TMR *rel_tmr = (AOS_RELAXED_TMR *)timer_ptr;

    AOS_ASSERT(rel_tmr);

    aos_deactivate_relaxed_timer(timer_ptr);

    rel_tmr->relaxed_time   = relaxed_time;
    rel_tmr->strict_time    = strict_time;
    rel_tmr->status         = RLX_TMR_IDLE;
    rel_tmr->b_period       = b_period;

    (void)tx_timer_change(rel_tmr->relaxed_timer, (ULONG)relaxed_time, 0);
    if (rel_tmr->strict_timer != NULL)
        (void)tx_timer_change(rel_tmr->strict_timer, (ULONG)strict_time, 0);

    aos_activate_relaxed_timer(timer_ptr);
}

/**
 * aos_get_relaxed_timer() - Get current relaxed timer.
 */
aos_rlx_timer_ptr aos_get_relaxed_timer(void)
{
    return (aos_rlx_timer_ptr)g_tmr_relaxed_list;
}

/**
 * aos_relaxed_timer_suspend() - Switch relaxed timer as strict timer.
 */
void aos_relaxed_timer_suspend(void)
{
    TX_TIMER_INTERNAL *tx_timer_ptr = (TX_TIMER_INTERNAL *)_tx_timer_list;
    AOS_RELAXED_TMR *next_timer, *rel_tmr = (AOS_RELAXED_TMR *)g_tmr_relaxed_list;
    TX_TIMER *tx_tmr;
    u32 i = 0;

    /* there is no timer or no relaxed timer, just return */
    if (tx_timer_ptr == NULL || rel_tmr == NULL)
        return;

    tx_timer_ptr = (TX_TIMER_INTERNAL *)_tx_timer_list;
    rel_tmr = (AOS_RELAXED_TMR *)g_tmr_relaxed_list;
    tx_tmr = (TX_TIMER *)rel_tmr->relaxed_timer;

    /* Suspend all the strict timers. */
    for (next_timer = rel_tmr;
         (next_timer != rel_tmr || i == 0);
         next_timer = next_timer->rel_next, i++) {
        aos_deactivate_timer(next_timer->relaxed_timer);

        if (next_timer->strict_time != AOS_WAIT_FOREVER && next_timer->strict_timer)
            aos_activate_timer(next_timer->strict_timer);

        next_timer->status = RLX_TMR_STRICT;
    }
}

/**
 * aos_relaxed_timer_resume() - Switch strict timer as relaxed timer.
 */
void aos_relaxed_timer_resume(void)
{
    TX_TIMER_INTERNAL *tx_timer_ptr = (TX_TIMER_INTERNAL *)_tx_timer_list;
    AOS_RELAXED_TMR *next_timer, *rel_tmr = (AOS_RELAXED_TMR *)g_tmr_relaxed_list;
    TX_TIMER *tx_tmr;
    u32 tick, i = 0;

     /* there is no timer or no relaxed timer, just return */
     if (rel_tmr == NULL)
         return;

     tx_tmr = (TX_TIMER *)rel_tmr->relaxed_timer;
     tick = aos_get_tickcount();

     /*
      * 1. the current timer is strict timer, and
      * 2. the interrupt is strict timer irq
      */
    for (next_timer = rel_tmr; (next_timer != rel_tmr || i == 0);
         next_timer = next_timer->rel_next, i++) {
         tx_tmr = (TX_TIMER *)next_timer->strict_timer;
        if (tx_timer_ptr == &tx_tmr->tx_timer_internal
#ifdef CONFIG_TIMER_TX_NO_TICK_IRQ
             && tx_timer_ptr->tx_timer_internal_ticks_onwall <= (ULONG64)tick
#endif
            )
            return;
    }

    /*
     * 1. the current timer is not strict timer, or
     * 2. the current timer is strict timer but it not timeout
     */
    for (next_timer = rel_tmr, i = 0; (next_timer != rel_tmr || i == 0);
         next_timer = next_timer->rel_next, i++) {
        /* Find the strict timer in the list and resume it. */
        if (next_timer->strict_timer)
            aos_deactivate_timer(next_timer->strict_timer);

        aos_activate_timer(next_timer->relaxed_timer);

        next_timer->status = RLX_TMR_REWORK;
    }

    return;
}

#ifdef CONFIG_FSMMSG_FEATURE
#ifdef WIN32
#define AOS_TIMER_THREAD_TID        24
#else
#include <ps_config.h>
#endif

static void _fsm_timer_timeout_func(void *input)
{
    TX_INTERRUPT_SAVE_AREA
    aos_thread_id thread_id;
    aos_fsm_msg *fsm_msg_ptr = AOS_NULL;
    TX_TIMER *tx_timer_ptr = (TX_TIMER *)input;

    AOS_ASSERT(tx_timer_ptr);
    TX_DISABLE
    fsm_msg_ptr = (aos_fsm_msg *)(tx_timer_ptr->extension_ptr);
    if (!fsm_msg_ptr) {
        /* may has been delete by stop or delete timer, just return. */
        TX_RESTORE
        return;
    }

    thread_id = FSM_DEST_THREAD_ID(fsm_msg_ptr);
    FSM_SRC_THREAD_ID(fsm_msg_ptr) = AOS_TIMER_THREAD_TID;
    FSM_SRC_PROCESS_ID(fsm_msg_ptr) = 0;

    /* For period timer: link fsm msg memory to avoid being freed */
    if (tx_timer_ptr->tx_timer_internal.tx_timer_internal_re_initialize_ticks)
        aos_link_fsm_msg(fsm_msg_ptr);
    /* For one shot timer: clear the msg ptr which saved in extension_ptr */
    else
        tx_timer_ptr->extension_ptr = 0;
    TX_RESTORE

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
#ifdef CONFIG_RTOS_THREADX
    TX_INTERRUPT_SAVE_AREA
    TX_TIMER *tx_timer_ptr = (TX_TIMER *)timer_ptr;
    u32 tick;

    AOS_ASSERT(tx_timer_ptr);

#ifdef CONFIG_CHIP_FPGA
    /* On fpga board, need enlarge the expiration time. */
    expiration_time = expiration_time *48;
#endif

    /* translate time(ms) to tick */
    tick = aos_time_to_tick(expiration_time);

    if (tx_timer_deactivate(tx_timer_ptr) != TX_SUCCESS)
        return AOS_ERROR;

    TX_DISABLE
    /* update msg ptr */
    tx_timer_ptr->extension_ptr = msg_ptr;
    /* update timeout value */
    tx_timer_ptr->tx_timer_internal.tx_timer_internal_remaining_ticks =
                                    tick;
    /* update re_initialize_ticks for period timer */
    if (tx_timer_ptr->tx_timer_internal.tx_timer_internal_re_initialize_ticks)
        tx_timer_ptr->tx_timer_internal.tx_timer_internal_re_initialize_ticks =
        tick;
    TX_RESTORE

    if (tx_timer_activate(tx_timer_ptr) != TX_SUCCESS)
        return AOS_ERROR;

    return AOS_SUCCESS;
#else
    return AOS_ERROR;
#endif
}

/**
* aos_stop_fsm_timer() - Stop fsm timer.
* @timer_ptr: Specified fsm timer pointer.
*/
void aos_stop_fsm_timer(aos_timer_ptr timer_ptr)
{
#ifdef CONFIG_RTOS_THREADX
    TX_INTERRUPT_SAVE_AREA
    TX_TIMER *tx_timer_ptr = (TX_TIMER *)timer_ptr;
    aos_fsm_msg *msg_ptr = AOS_NULL;
    UINT ret;

    AOS_ASSERT(tx_timer_ptr);

    /* deactive timer */
    ret = tx_timer_deactivate(tx_timer_ptr);
    AOS_ASSERT(AOS_SUCCESS == ret);

    TX_DISABLE
    /* free fsm msg ptr */
    msg_ptr = (aos_fsm_msg *)(tx_timer_ptr->extension_ptr);
    if (!msg_ptr) {
        TX_RESTORE
        return;
    }

    tx_timer_ptr->extension_ptr = 0;
    TX_RESTORE

    aos_free_fsm_msg(msg_ptr);
#else

#endif
}

/**
* aos_delete_fsm_timer() - Delete specified fsm timer.
* @timer_ptr: Specified fsm timer ptr.
*/
void aos_delete_fsm_timer(aos_timer_ptr timer_ptr)
{
#ifdef CONFIG_RTOS_THREADX
    TX_INTERRUPT_SAVE_AREA
    TX_TIMER *tx_timer_ptr = (TX_TIMER *)timer_ptr;
    aos_fsm_msg *msg_ptr = AOS_NULL;
    UINT ret;

    AOS_ASSERT(tx_timer_ptr);

    TX_DISABLE
    /* free fsm msg ptr */
    msg_ptr = (aos_fsm_msg *)(tx_timer_ptr->extension_ptr);
    if (msg_ptr) {
        aos_free_fsm_msg(msg_ptr);
        tx_timer_ptr->extension_ptr = 0;
    }
    TX_RESTORE

    /* All errors are caused by improper use this api, assert it for debug. */
    ret = tx_timer_delete(tx_timer_ptr);
    AOS_ASSERT(ret == AOS_SUCCESS);

    /* timer_ptr is malloc when created, must free it here */
    aos_free(tx_timer_ptr);

#endif
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
    FSM_SRC_THREAD_ID(fsm_msg_ptr) = AOS_TIMER_THREAD_TID;
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
#ifdef CONFIG_RTOS_THREADX
    TX_INTERRUPT_SAVE_AREA
    TX_TIMER *tx_timer_ptr;
    u32 tick;

    AOS_ASSERT(ptr);
    tx_timer_ptr = (TX_TIMER *)ptr->timer;

    if (tx_timer_deactivate(tx_timer_ptr) != TX_SUCCESS)
        return AOS_ERROR;

    /* translate time(ms) to tick */
    tick = aos_time_to_tick(expiration_time);

    TX_DISABLE
    ptr->b_stop = FALSE;

    /* update timeout value */
    tx_timer_ptr->tx_timer_internal.tx_timer_internal_remaining_ticks = tick;
    /* update re_initialize_ticks for period timer */
    if (tx_timer_ptr->tx_timer_internal.tx_timer_internal_re_initialize_ticks)
        tx_timer_ptr->tx_timer_internal.tx_timer_internal_re_initialize_ticks =
        tick;
    TX_RESTORE

    if (tx_timer_activate(tx_timer_ptr) != TX_SUCCESS)
        return AOS_ERROR;

    return AOS_SUCCESS;
#else
    return AOS_ERROR;
#endif
}


/**
* aos_stop_fsm_timer() - stop a fsmmsg timer.
* @ptr: the aos_fsmmsgtimer ptr that createed by aos_create_fsmmsg_timer.
*/
void aos_stop_fsmmsg_timer(aos_fsmmsgtimer *ptr)
{
#ifdef CONFIG_RTOS_THREADX
    TX_INTERRUPT_SAVE_AREA
    TX_TIMER *tx_timer_ptr;
    UINT ret;

    AOS_ASSERT(ptr);
    TX_DISABLE
    ptr->b_stop = TRUE;
    TX_RESTORE

    tx_timer_ptr = (TX_TIMER *)ptr->timer;

    /* deactive timer */
    ret = tx_timer_deactivate(tx_timer_ptr);
    AOS_ASSERT(AOS_SUCCESS == ret);
#else

#endif
}

/**
* aos_delete_fsm_timer() - delete a fsmmsg timer.
* Will both delete the timer and the msg.
* @ptr: the aos_fsmmsgtimer ptr that createed by aos_create_fsmmsg_timer.
*/
void aos_delete_fsmmsg_timer(aos_fsmmsgtimer *ptr)
{
#ifdef CONFIG_RTOS_THREADX
    TX_TIMER *tx_timer_ptr;
    UINT ret;

    AOS_ASSERT(ptr);

    aos_free_fsm_msg(ptr->msg);

    tx_timer_ptr = (TX_TIMER *)ptr->timer;
    ret = tx_timer_delete(tx_timer_ptr);
    AOS_ASSERT(ret == AOS_SUCCESS);

    /* tx_timer_ptr is malloc in aos_create_fsmmsg_timer()->_create_timer(), must free it here */
    aos_free(tx_timer_ptr);

    aos_free(ptr);
#endif
}
#endif

