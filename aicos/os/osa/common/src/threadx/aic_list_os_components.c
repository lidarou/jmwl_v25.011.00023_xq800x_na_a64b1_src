/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is used to list the aic threadx components.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x04cd8d
#define LOG_MODULE    AMOD_AOS

#include <aic_cmd_api.h>
#include <aic_list_os_components.h>
#include <aic_type.h>
#include <tx_api.h>
#include <tx_event_flags.h>
#include <tx_mutex.h>
#include <tx_queue.h >
#include <tx_semaphore.h>
#include <tx_thread.h>
#include <tx_timer.h>

/**
* All the components usd the macro AIC_TX_COMP_LIST to list temporary.
*/

#define COMP_GET_CREATE(ptr) \
    ptr = _tx_##comp##_created_ptr;

#define COMP_GET_NEXT(ptr, comp) \
    ptr->tx_##comp##_created_next

#define COMP_GET_MEMBER(ptr, comp, member) \
    ptr->tx_##comp##_##member

#define AIC_TX_COMP_LIST(ptr, comp, cnt, func) do \
{ \
    ptr = _tx_##comp##_created_ptr; \
 \
    while (ptr) { \
        CONS_OUT("%d. %s:\n", cnt,\
                 COMP_GET_MEMBER(ptr, comp, name)); \
        func(ptr); \
        CONS_OUT("\r\n"); \
        cnt++; \
        ptr = COMP_GET_NEXT(ptr, comp); \
        if (ptr == _tx_##comp##_created_ptr) \
            break;\
    } \
} while (0)


void aic_comp_list_thread(TX_THREAD *ptr)
{
    CONS_OUT("run_cnt=%d, stack ptr=0x%p, highest=0x%p\n",
             ptr->tx_thread_run_count,
             ptr->tx_thread_stack_ptr,
             ptr->tx_thread_stack_highest_ptr);

    CONS_OUT("stack size=0x%x, start=0x%p, end=0x%p, end-highest=0x%x\n",
             ptr->tx_thread_stack_size,
             ptr->tx_thread_stack_start,
             ptr->tx_thread_stack_end,
             (ulong)ptr->tx_thread_stack_end -
             (ulong)ptr->tx_thread_stack_highest_ptr);

    CONS_OUT("state =%d, priority=%d, preempt threshold=%ud\n",
             ptr->tx_thread_state,
             ptr->tx_thread_user_priority,
             ptr->tx_thread_preempt_threshold);
}

void aic_comp_list_event(TX_EVENT_FLAGS_GROUP *ptr)
{

}

void aic_comp_list_mutex(TX_MUTEX *ptr)
{

}

void aic_comp_list_semaphore(TX_SEMAPHORE *ptr)
{

}

void aic_comp_list_queue(TX_QUEUE *ptr)
{

}

void aic_comp_list_timer(TX_TIMER *ptr)
{

}

int aic_comp_list(AIC_COMPONENTS components) {
    int cnt = 0;
    TX_EVENT_FLAGS_GROUP *_tx_event_flags_group_created_ptr =
        _tx_event_flags_created_ptr;

    switch (components) {
    case AIC_COMP_THREAD:
    {
        TX_THREAD *ptr;

        AIC_TX_COMP_LIST(ptr, thread, cnt, aic_comp_list_thread);
        break;
    }

    case AIC_COMP_EVENT:
    {
        TX_EVENT_FLAGS_GROUP *ptr;

        AIC_TX_COMP_LIST(ptr, event_flags_group, cnt, aic_comp_list_event);
        break;
    }

    case AIC_COMP_MUTEX:
    {
        TX_MUTEX *ptr;

        AIC_TX_COMP_LIST(ptr, mutex, cnt, aic_comp_list_mutex);
        break;
    }

    case AIC_COMP_SEMAPHORE:
    {
        TX_SEMAPHORE *ptr;

        AIC_TX_COMP_LIST(ptr, semaphore, cnt, aic_comp_list_semaphore);
        break;
    }

    case AIC_COMP_QUEUE:
    {
        TX_QUEUE * ptr;

        AIC_TX_COMP_LIST(ptr, queue, cnt, aic_comp_list_queue);
        break;
    }

    case AIC_COMP_TIMER:
    {
        TX_TIMER *ptr;

        AIC_TX_COMP_LIST(ptr, timer, cnt, aic_comp_list_timer);
        break;
    }

    default:
        cnt = 0;
    }

    return cnt;
}
