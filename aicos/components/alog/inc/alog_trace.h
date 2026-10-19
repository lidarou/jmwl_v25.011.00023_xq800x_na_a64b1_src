/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log trace header file.
 */
#ifndef __ALOG_TRACE_H__
#define __ALOG_TRACE_H__

#include <aipc_api.h>
#include <alog_buffer.h>
#include <alog_formater.h>

/**
 * ALOG_TRACE- alog trace data define.
 *
 * @brr: used to transmit the trace.
 * @buf_mgr: used to manage the trace buffer.
 * @trace_formater: use to format a tarce buffer in to log frame.
*/
typedef struct ALOG_TRACE {
    ABRR_HANDLE brr;
    ALBF_MGR_T  *buf_mgr;
    ALFM_T      *trace_formater;
} ALOG_TRACE_T;

ALOG_TRACE_T *alog_trace_create(ABRR_HANDLE brr);
void alog_trace_destroy(ALOG_TRACE_T *alog_trace);
void alog_trace_flush_boot_log(void);
void alog_uart_log_handle_init(void);
#endif /* __ALOG_TRACE_H__ */

