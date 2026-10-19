/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the apis that used by tx timer..
 */

#ifndef __TIMER_TX_INC_H__
#define __TIMER_TX_INC_H__

#include <aic_type.h>

#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif
#ifndef WIN32
#include <user_tracex_api.h>
#endif

typedef enum  {
    TX_TMR_DBG_SYS_ACTIVE = 1,
    TX_TMR_DBG_SYS_DEACTIVE = 2,
    TX_TMR_DBG_DEACTIVE = 3,
    TX_TMR_DBG_SHELL_S = 4,
    TX_TMR_DBG_SHELL_E = 5,
    TX_TMR_DBG_SET_EXP = 6,
    TX_TMR_DBG_CLR_EXP = 7,
    TX_TMR_DBG_IRQ = 8,
    TX_TMR_DBG_CHG = 9
} TX_TMR_DGB_ID;

#ifdef CONFIG_TIMER_TX_NO_TICK_IRQ
ULONG _tx_timer_get_remaining(TX_TIMER_INTERNAL *timer_ptr);
VOID _tx_timer_set_expire(TX_TIMER_INTERNAL *timer_ptr);
VOID _tx_timer_clear_expire(void);
ULONG64 _tx_timer_get_tick(VOID);
#ifdef CONFIG_TIMER_TX_NO_TICK_DBG
VOID _tx_timer_dbg(UINT id, TX_TIMER_INTERNAL *timer_ptr);
#define TX_TIMER_DBG(i, t)      _tx_timer_dbg(i, t)
#else
#ifdef TX_ENABLE_EVENT_TRACE
#define TX_TIMER_DBG(i, t)\
    {\
        TX_THREAD *thread_ptr;\
        TX_TIMER_INTERNAL *tmr_ptr = (TX_TIMER_INTERNAL *)t;\
        ULONG fn_name = 0, onwall = 0;\
        \
        if (tmr_ptr) {\
            thread_ptr = container_of(tmr_ptr, TX_THREAD, tx_thread_timer);\
            if (thread_ptr->tx_thread_id == TX_THREAD_ID)\
                fn_name = (UINT)thread_ptr->tx_thread_name;\
            else\
                fn_name = (UINT)tmr_ptr->tx_timer_internal_timeout_function;\
            onwall = (ULONG)tmr_ptr->tx_timer_internal_ticks_onwall;\
        }\
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_TIMER_EVENT, i, fn_name, onwall, 0, TX_TRACE_USER_EVENTS);\
    }
#else
#define TX_TIMER_DBG(i, t)
#endif
#endif
#endif

#endif /* __TIMER_TX_INC_H__ */

