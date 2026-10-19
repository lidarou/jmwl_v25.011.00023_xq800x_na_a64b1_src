/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log module main source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0fc80e
#define LOG_MODULE    AMOD_ALOG

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_size.h>
#include <aic_math.h>
#include <aic_io_dev.h>
#include <aipc_api.h>
#include <alog_api.h>
#include <aos_thread.h>
#include <aos_config.h>
#include <alog_inner.h>
#include <alog_trace_filter.h>
#include <alog_trace_format.h>
#include <alog_frame.h>
#include <alog_formater.h>
#include <alog_trace.h>
#include <stdio.h>
#include <stdarg.h>
#include <connmgr_api.h>
#include <adbg_cfg_api.h>
#include <serial_inc.h>
#include <alog_api_fast.h>

#define ALTRACE_BUF_MGR_NAME  "trbm"

ALBF_MGR_T *s_log_buf;

#if IS_CONF_ENABLED(ALOG_EARLY_BOOY_LOG)
/**
 * s_mux_buf:
 * if defined AOS_CONFIG_TRACEX
 * It is multiplexed with trace_buffer.
 */
#ifdef AOS_CONFIG_TRACEX
extern u32 trace_buffer[];
#define ALOG_EARLY_TRACE_SIZE   AOS_TRACE_BUFFER_SIZE
static u8* const s_mux_buf = (u8 *)trace_buffer;
#else
#define ALOG_EARLY_TRACE_SIZE   SIZE_2K
static u8 s_mux_buf[ALOG_EARLY_TRACE_SIZE];
#endif

static u16 s_mux_buf_used;

static void alog_flush_eraly_boot_log(void)
{
    u16 left = s_mux_buf_used;
    u8 *cur = s_mux_buf;
    ALBF_BLK_T blk;

    while (left) {
        /* use min size to try. */
        blk.size = ALOG_BUF_ONCE_MIN;
        albf_mgr_get_blk(s_log_buf, &blk);

        /* adjust the blk.size. */
        blk.size = MIN(left, blk.size);
        memcpy(blk.buf, cur, blk.size);
        albf_mgr_put_blk(s_log_buf, &blk);

        cur += blk.size;
        left -= blk.size;
    }
}

static void alog_trace_put_blk_log(ALBF_BLK_T *blk)
{
#ifdef WIN32
    u32 size;
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE

    for (size = 0; size < blk->size; size++)
        putchar(blk->buf[size]);

    AOS_INTERRUPT_RESTORE
#else
#if IS_CONF_ENABLED(ALOG_TRACE_UART_DIRECT)
    if (s_log_uart) {
#ifdef CONFIG_ALOG_FRAME_SUPPORT
        u8 header[ALOG_FRAME_H_SIZE];
        static u32 s_log_sn;

        alog_trace_format(s_log_sn++, header, blk->size + sizeof(header));
        uart_write_dir(s_log_uart, header, sizeof(header));
#endif
        uart_write_dir(s_log_uart, blk->buf, blk->size);
    }
#endif
#endif /* WIN32 */
}
#endif

PLAT_TCM_CODE static bool alog_filter_or_get_blk_failed(ALBF_BLK_T *blk, u32 mod,
                                          u32 level, size_t size,
                                          bool *need_flush)
{
    /* check trace enable. */
    if (!alog_is_trace_enable())
        return TRUE;

    /* check trace filter. */
    if (alog_trace_filter_out(mod, level))
        return TRUE;

    /* if s_log_buf is ready, use it, else use the boot buffer. */
    if (s_log_buf) {
        blk->size = size;
        *need_flush = albf_mgr_get_blk(s_log_buf, blk);
    } 
    else {
#if IS_CONF_ENABLED(ALOG_EARLY_BOOY_LOG)
        *need_flush = FALSE;
        blk->buf = s_mux_buf + s_mux_buf_used;
        blk->size = ALOG_EARLY_TRACE_SIZE - s_mux_buf_used;
        /* size not big enough, lost log. */
        if (blk->size < size)
            return TRUE;
#else
        return TRUE;
#endif
    }

    return FALSE;
}

/**
 * UPDATE_AND_PUT_ABLK -
 * update the actual size of the blk and put it to blk pool.
 */
#if IS_CONF_ENABLED(ALOG_EARLY_BOOY_LOG)
#define UPDATE_AND_PUT_ABLK(blk, actual_size) do { \
        (blk).size = (actual_size); \
        if (s_log_buf) { \
            albf_mgr_put_blk(s_log_buf, &(blk)); \
        } else { \
            if (s_mux_buf_used + (actual_size) < ALOG_EARLY_TRACE_SIZE) \
                s_mux_buf_used += (actual_size); \
        } \
    } while(0)
#else
#define UPDATE_AND_PUT_ABLK(blk, actual_size) do { \
        (blk).size = (actual_size); \
        if (s_log_buf) { \
            albf_mgr_put_blk(s_log_buf, &(blk)); \
        } \
    } while(0)
#endif

#ifdef WIN32
/**
 * SNPRINTF_COMP_WIN32 - snprintf compatible with win32.
 *
 * if the buffer size is not big enough vsnprintf will return -1.
 * so must call _vscprintf to get the format string len.
 */
#define SNPRINTF_COMP_WIN32 \
do { \
        va_start(args, fmt); \
        len = _vscprintf(fmt, args); \
        va_end(args); \
        if (len < 0) { \
            AOS_INTERRUPT_RESTORE \
            return; \
        } \
        blk.size += (size_t)len; \
    } while (0)
#else
#define SNPRINTF_COMP_WIN32
#endif /* WIN32 */

/**
 * ADD_TICK_AND_MOD_NAME - get the ticks and module name of the trace.
 */

#define ADD_TICK_AND_MOD_NAME \
    snprintf((char *)blk.buf, blk.size, "%04lu %s: ", \
                   alog_get_aon_timestamp(), aic_get_moudule_name(mod))

#if IS_CONF_ENABLED(ALOG_TRACE_UART_DIRECT)
void *s_log_uart;

void alog_uart_log_handle_init(void)
{
    int log_com ;
    AIO_DEV_T *dev;

#ifdef CONFIG_CONNECT_MGR
    log_com = conn_get_log_dev_com(AIC_IODEV_UART);
#else
    log_com = adbg_get_log_dev_com();
#endif

    if (log_com >= 0) {
        dev = aio_request_dev(AIC_IODEV_UART, log_com);
        s_log_uart = aio_get_dev_uart_handle(dev);
    }

#if IS_CONF_ENABLED(ALOG_EARLY_BOOY_LOG)
    if (s_log_uart)
        alog_trace_flush_boot_log();
#endif
}
#endif

#ifdef CONFIG_ALOG_TRACE_IDX
#include <alog_idx_api_do.h>
#else
#include <alog_trace_api_do.h>
#endif

#if IS_CONF_ENABLED(ALOG_EARLY_BOOY_LOG)
void alog_trace_flush_boot_log(void)
{
    ALBF_BLK_T blk;

    if (s_mux_buf_used) {
        blk.buf = s_mux_buf;
        blk.size = s_mux_buf_used;
        alog_trace_put_blk_log(&blk);
    }
}
#endif

ALOG_TRACE_T *alog_trace_create(ABRR_HANDLE brr)
{
    const ALOG_BUF_CFG_T *buf_cfg;
    ALOG_TRACE_T *alog_trace;

    aicx_info(0x0105b6, "", "creat trace\n");

    alog_trace = aos_malloc(sizeof(*alog_trace));
    AIC_ASSERT(alog_trace);

    alog_trace->brr = brr;
    buf_cfg = alog_get_buf_cfg(ALOG_TYPE_TRACE);

    /* creat trace_formater */
    alog_trace->trace_formater = alfm_create(ALFMT_TRACE_FORMAT);

    /* creat buf_mgr */
    alog_trace->buf_mgr =
        albf_mgr_create(ALTRACE_BUF_MGR_NAME, (int)buf_cfg->priority,
                        brr, ALTRACE_LINK, buf_cfg, alog_trace->trace_formater);
    s_log_buf = alog_trace->buf_mgr;

#if IS_CONF_ENABLED(ALOG_EARLY_BOOY_LOG)
    /* flush eraly boot log. */
    alog_flush_eraly_boot_log();
#endif

    return alog_trace;
}

void alog_trace_destroy(ALOG_TRACE_T *alog_trace)
{
    if (!alog_trace)
        return;

    /* destroy buf_mgr */
    s_log_buf = NULL;
    albf_mgr_destroy(alog_trace->buf_mgr);

    /* destroy alfmt */
    alfm_destroy(alog_trace->trace_formater);

    /* free alog trace */
    aos_free(alog_trace);
}

