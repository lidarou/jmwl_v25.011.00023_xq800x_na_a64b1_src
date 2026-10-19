/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log buffer header file.
 */
#ifndef __ALOG_BUF_H__
#define __ALOG_BUF_H__
#include <aic_osa.h>
#include <aic_size.h>
#include <aic_type.h>
#include <aipc_api.h>
#include <alog_cfg.h>
#include <alog_inner.h>
#include <alog_formater.h>
#include <alog_outputer.h>

/*
 * Assume the minimum size of log once.
 * If the remaining size of buffer is less than this size,
 * use another buffer.
 */
#define ALOG_BUF_ONCE_MIN   100

/* ALOG_BUF_BLK - use to return the caller a available area. */
typedef struct ALOG_BUF_BLK {
    u8 *buf;
    size_t size;
} ALBF_BLK_T;

/**
 * ALOG_BUF - a flush buf.
 * @sn:  the sn of the buffer.
 * @size:the size of the buffer.
 * @wt_size: have write size of the buffer.
 * @buf: buffer pointer.
 */
typedef struct ALOG_BUF {
    u32     sn;
    size_t  size;
    size_t  wt_size;
    u8      *buf;
} ALOG_BUF_T;

/**
 * ALOG_BUF_MGR - the buffer manager.
 * @buf_cnt: the buffer cnt of the manager.
 * @offset: Loop over the number of buffers that need to skip for boot log.
 * @rd_index: the current read buffer index.
 * @wt_index: the current write buffer index.
 * @link_id: the linkid of the buffer manager.
 *           It means that which link of the log brr used to send buffer log.
 *           such as trace log buffer manager use a link to send the trace log,
             message log buffer mananger use another link to send the message log.
 * @buffer_busy: the buffer is busy, can't be written.
 * @overtake: if happend wt_index overtake rd_index.
 * @wait_remote: if it is waiting remote.
 * @sn: save the latest sn, the range is [1, 0xffffffff].
 * @read_sn: the sn of the latest read buffer.
 * @write_sn:the sn of the latest write buffer.
 */
typedef struct ALOG_BUF_MGR {
    u16  buf_cnt;
    u16  offset;
    u16 volatile  rd_index;
    u16 volatile  wt_index;
    u8  link_id;
    u8  buffer_busy;
    u8  overtake;
    u8  wait_remote;
    u8  wait_remote_cnt;

#if IS_CONF_ENABLED(ALOG_TRACE_LOG_STATISTICS)
    u32 last_tick;
    u32 stati_size;
    u32 max_rate;
#endif

    u32 volatile sn;
    u32 volatile read_sn;
    u32 volatile write_sn;
    u32 slp_handle;

    char *name;
    ABRR_HANDLE log_brr;
    ALFM_T      *alfm;
    size_t      buf_size;
    size_t      head_size;
    ALOG_BUF_T  *bufs;
    ALOG_BUF_T  *cur_buf;
    ALOP_T      *alop;
#ifdef ALOG_FLUSH_LOG_TIMER
    aos_timer_ptr       flush_timer;
#endif
    aos_semaphore_ptr   sema;
    aos_thread_id       thread;
} ALBF_MGR_T;

ALBF_MGR_T *albf_mgr_create(char *buf_mgr_name, int buf_mgr_pri,
                            ABRR_HANDLE log_brr, u8 link_id,
                            const ALOG_BUF_CFG_T *buf_cfg, ALFM_T *lfm);
void albf_mgr_destroy(ALBF_MGR_T *albf_mgr);

/**
 * albf_mgr_get_blk - get a block form buffer mgr, if success, must update the
 * actual used size and put it back with function albf_mgr_put_blk.
 *
 * @mgr: the instance of the buffer manager.
 * @blk: both input and output parameters.
 *       input, blk->size is the request size.
 *       output, blk->size is the block size, blk->buf is the block address.
 *
 * no need return value, success or assert.
 */
PLAT_TCM_CODE bool albf_mgr_get_blk(ALBF_MGR_T *albf_mgr,  ALBF_BLK_T *blk);
/**
 * albf_mgr_get_blk_again -The left space of current buffer is not enough,
 * flush the current buffer and jump to the next buffer. Before call this
 * function, the user must has call the albf_mgr_get_blk, and found left space
 * in old buffer is not enough.
 */
 
PLAT_TCM_CODE bool albf_mgr_get_blk_again(ALBF_MGR_T *mgr, ALBF_BLK_T *blk);
/**
 * albf_mgr_notify_flush_task -Put a semaore to notify the flush task to
 * flush the log to output task.
 */
void albf_mgr_notify_flush_task(ALBF_MGR_T *mgr);

/**
 * albf_mgr_put_blk - get a blk form buffer mgr, if success, must update the
 * actual used size and put it back with function albf_mgr_put_blk.
 *
 * @mgr: the instance of the buffer manager.
 * @blk: both input parameters, blk->size is the actual used size.
 */
PLAT_TCM_CODE void albf_mgr_put_blk(ALBF_MGR_T *albf_mgr, ALBF_BLK_T * blk);

/**
 * albf_mgr_flush_buf_in_assert - flush all the buffer log after system assert.
 */
void albf_mgr_flush_all_buf_in_assert(ALBF_MGR_T *mgr);

/**
 * albf_mgr_output_block_in_assert - output a blk directly after system assert.
 */
void albf_mgr_output_block_in_assert(ALBF_MGR_T *mgr, u8 link_id, ABLK_T *blk);

/**
 * albf_mgr_set_output - set the alop of the buffer mgr for output
 * the trace in buffer after system assert.
 */
void albf_mgr_set_output(ALBF_MGR_T *mgr, ALOP_T *alop);

#endif /* __ALOG_BUF_H__ */

