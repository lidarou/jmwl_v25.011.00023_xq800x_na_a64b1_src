/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** ThreadX Component USER tracex                                         */
/**                                                                       */
/**   User Specific                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#ifndef _USER_TRACE_API_H
#define _USER_TRACE_API_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include <tx_user.h>
#include <tx_port.h>
#include <tx_api.h>

/* Determine if tracing is enabled.  */

#if defined(TX_ENABLE_EVENT_TRACE)

/* Trace is enabled. Remap calls so that interrupts can be disabled around the actual event logging.  */
#include "tx_trace.h"

/* Define the USER TRACEX events first.
   User-defined event numbers start at 4096 and continue through 65535, as defined by the constants
   TX_TRACE_USER_EVENT_START and TX_TRACE_USER_EVENT_END, respectively. User events should be based
   on these constants in case the user event number assignment is changed in future releases.  */


/* Map the trace macros to USER TRACE. Please Make sure your event id is unique */
#define TX_TRACE_MCX_EVENT          (TX_TRACE_USER_EVENT_START)     /* 4096 */
#define TX_TRACE_SLP_EVENT          (TX_TRACE_USER_EVENT_START + 1) /* 4097 */
#define TX_TRACE_SNOR_EVENT         (TX_TRACE_USER_EVENT_START + 2) /* 4098 */
#define TX_TRACE_FMD_EVENT          (TX_TRACE_USER_EVENT_START + 3) /* 4099 */
#define TX_TRACE_LX_EVENT           (TX_TRACE_USER_EVENT_START + 4) /* 4100 */
#define TX_TRACE_FTL_EVENT          (TX_TRACE_USER_EVENT_START + 5) /* 4101 */
#define TX_TRACE_ASSERT_EVENT       (TX_TRACE_USER_EVENT_START + 6) /* 4102 */
#define TX_TRACE_DVFS_EVENT         (TX_TRACE_USER_EVENT_START + 7) /* 4103 */
#define TX_TRACE_TIMER_EVENT        (TX_TRACE_USER_EVENT_START + 8) /* 4104 */
#define TX_TRACE_SSP_EVENT          (TX_TRACE_USER_EVENT_START + 9) /* 4105 */
#define TX_TRACE_SMD_EVENT          (TX_TRACE_USER_EVENT_START + 10) /* 4106 */
#define TX_TRACE_PSRAM_EVENT        (TX_TRACE_USER_EVENT_START + 11) /* 4107 */
#define TX_TRACE_NETWORK_EVENT      (TX_TRACE_USER_EVENT_START + 12) /* 4108 */
#define TX_TRACE_INTDBG_EVENT       (TX_TRACE_USER_EVENT_START + 13) /* 4109 */

/*USRX Define for tracex*/
#define USER_TRACE_IN_LINE_INSERT(id, a, b) tx_trace_user_event_insert((ULONG)id, (ULONG)a, (ULONG)b, (ULONG)0, (ULONG)0);
#define USER_TRACE_IN_LINE_INSERT_EX(id, a, b, c, d) tx_trace_user_event_insert((ULONG)id, (ULONG)a, (ULONG)b, (ULONG)c, (ULONG)d);
#else
#define USER_TRACE_IN_LINE_INSERT(id, a, b)
#define USER_TRACE_IN_LINE_INSERT_EX(id, a, b, c, d)
#endif


/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef __cplusplus
}
#endif

#endif


