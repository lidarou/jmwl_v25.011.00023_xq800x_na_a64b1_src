/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The alog private header file.

 */

#ifndef __ALOG_INNER_H__
#define __ALOG_INNER_H__

#include <aic_errno.h>
#include <aic_io.h>
#include <aic_osa.h>
#include <aic_math.h>
#include <aic_size.h>
#include <aic_list.h>
#include <alog_api.h>
#include <stdio.h>

#ifndef WIN32
#include <timer_inc.h>
#endif

/*
 * ALOG_TRACE_UART_DIRECT -
 * open it will printf log at the context of the caller.
 */
#define ALOG_TRACE_UART_DIRECT  0

/*
 * ALOG_EARLY_BOOY_LOG -
 * open it will printf the early boot log.
 */
//#define ALOG_EARLY_BOOY_LOG     0

/*
 * ALOG_TRACE_LOST_LOG_DUMP
 * open it will assert the cp and ap when lost log.
 */
#define ALOG_TRACE_LOST_LOG_DUMP  0

/*
 * ALOG_TRACE_LOST_LOG_NOT_WAIT_REMOTE
 * open it lost log will assert, even wait remote is false.
 */
#define ALOG_TRACE_LOST_LOG_NOT_WAIT_REMOTE  0

/*
 * ALOG_TRACE_LOG_STATISTICS
 * open it will produce the log statistics info.
 */
#define ALOG_TRACE_LOG_STATISTICS  0

#if defined (CONFIG_SUBSYS_CP)
#define ALOG_EARLY_BOOY_LOG     0
#else
#define ALOG_EARLY_BOOY_LOG     1
#endif

#if defined (CONFIG_SUBSYS_AP) || defined(WIN32) || \
    defined (CONFIG_CHIP_FPGA)
/*
 * ALOG_FLUSH_LOG_TIMER
 * In ap open it for flush log.
 */
#define ALOG_FLUSH_LOG_TIMER
#endif


/* how long timer do once statistics, the unit is ms. */
#define  ALOG_TRACE_STATI_ONCE_TIMES    100
#define  ALOG_TRACE_STATI_ONE_TICK_US   30
#define  MAX_32BIT_DIV_1000     0x4294967 /* 0xFFFFFFFF/1000 */
#define  ALOG_TRACE_STATI_ONCE_TICKS    \
    (ALOG_TRACE_STATI_ONCE_TIMES * 1000 / ALOG_TRACE_STATI_ONE_TICK_US)

/*
 * USE_DMA_CPY -
 * open it will copy data log and msg use dma.
 */
#ifndef WIN32
#define USE_DMA_CPY     0
#endif

/* temp diasable ALOG_ADSP_AIDX */
#define ALOG_ADSP_AIDX  1

/* alog type define */
enum {
    ALOG_TYPE_TRACE= 0,
    ALOG_TYPE_MESSAGE,
    ALOG_TYPE_DATA,
};

/* alog data type define */
enum {
    ALD_TYPE_DUMP = 0,
    ALD_TYPE_CAP,
    ALD_TYPE_AUDIO
};

/**
* alog_get_aon_timestamp() - get the aon timestap, ap and cp use the same timestamp.
*
* Return: current tickcout (30.5us).
*/
#define alog_get_aon_timestamp()   aos_get_aon_raw_tick()

#endif /* __ALOG_INNER_H__ */

