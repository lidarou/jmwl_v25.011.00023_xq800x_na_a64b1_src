/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines Threadx configurations.
 */

#ifndef __TX_CONFIG_H__
#define __TX_CONFIG_H__


/* define ticks per second. this value = 1000/TX_TIMER_PERIODIC */
#define TX_TIMER_TICKS_PER_SECOND        100

/* Define the priority levels for ThreadX.  Legal values range
   from 32 to 1024 and MUST be evenly divisible by 32. */
#define TX_MAX_PRIORITIES                256

/* Determine if the trace event logging code should be enabled. */
#define TX_ENABLE_EVENT_TRACE
extern volatile unsigned long            _tx_timer_system_clock;
#define TX_TRACE_TIME_SOURCE             _tx_timer_system_clock

#define CONFIG_NX_BSD

/* add bsd_errno for netx */
#ifdef CONFIG_NX_BSD
#define TX_THREAD_USER_EXTENSION int bsd_errno;
#else
#define TX_THREAD_USER_EXTENSION
#endif

/* Determine whether or not stack checking is enabled */
//#define TX_ENABLE_STACK_CHECKING

#endif /* __TX_CONFIG_H__ */
