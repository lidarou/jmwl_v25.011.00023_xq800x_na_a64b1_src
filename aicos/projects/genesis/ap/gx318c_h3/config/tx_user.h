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

#define TX_SINGLE_MODE_SECURE

#ifdef CONFIG_TIMER_TX_NO_TICK_IRQ
#define TX_TIMER_TICKS_PER_SECOND        1000
#endif

/* Define the priority levels for ThreadX.  Legal values range
   from 32 to 1024 and MUST be evenly divisible by 32. */
#define TX_MAX_PRIORITIES                256

/* Determine if the trace event logging code should be enabled. */
#define TX_ENABLE_EVENT_TRACE

/* add bsd_errno for netx */
#ifdef CONFIG_NX_BSD
#define TX_THREAD_USER_EXTENSION int bsd_errno;
#else
#define TX_THREAD_USER_EXTENSION
#endif

#define TX_TRACE_TIME_SOURCE            *((ULONG *) 0x400b0000)

#endif /* __TX_CONFIG_H__ */
