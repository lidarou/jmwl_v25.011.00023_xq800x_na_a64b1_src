/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log buffer source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x19a011
#define LOG_MODULE    AMOD_ALOG

#include <adbg_cfg_api.h>
#include <diag_cmd_api.h>
#include <alog_buffer.h>
#include <alog_inner.h>
#include <alog_trace_format.h>
#include <slp_inc.h>
#include <aic_cmdline.h>
#include <ambox_api.h>

#define LOG_BUF_STATCK_SIZE 1024
#define LOG_BUF_FLUSH_TIME  1000

/**
 * albf_alloc_buf - get a new block from log brr.
 */
static int albf_alloc_buf(ALBF_MGR_T *mgr, ALOG_BUF_T *buf,
                          size_t buf_size, int timeout)
{
    int ret;
    ABLK_T blk;

    ret = ablk_get_block(mgr->log_brr, buf_size, &blk, timeout);
    if (ret)
        return ret;

    buf->buf = blk.addr;
    buf->size = blk.size;

    return AIC_SUCCESS;
}

/**
 * albf_free_buf - put the block back into log brr.
 */
static void albf_free_buf(ALBF_MGR_T *mgr, ALOG_BUF_T *buf)
{
    ABLK_T blk;

    blk.addr = (void*)buf->buf ;
    ablk_recycle_block(mgr->log_brr, &blk);
}

/**
 * albf_flush_buf - Flush all the logs in the buffer, and put the old block
 * of the buffer into the brr fifo, than get a new block for the buffer.
 */
#ifdef CONFIG_SYS_HIGH_PERF
#define albf_flush_buf(mgr, buf, save) \
do { \
    u32 sn = buf->sn; \
    int alloc, send; \
    ABLK_T blk; \
 \
    if (buf->wt_size <= mgr->head_size) { \
        mgr->read_sn = sn; \
        break; \
    } \
 \
    /* set the sn to 0 to indicate that it is flushed. */ \
    buf->sn = 0; \
 \
    /* if sn of the buffer is old than read sn, skip the buffer. */ \
    if (sn <= mgr->read_sn) \
        break; \
 \
    /* set buffer busy */ \
    mgr->buffer_busy = 1; \
 \
    if (mgr->read_sn + 1 != sn) \
        alog_info("%s: lost %d, exp sn=%d, read sn =%d", \
                  mgr->name, sn - (mgr->read_sn + 1), mgr->read_sn + 1, sn);\
 \
    /* updata the read sn. */ \
    mgr->read_sn = sn; \
 \
    /* get the flush block for send later. */ \
    blk.addr = (void*)buf->buf; \
    blk.size = buf->wt_size; \
    buf->sn = 0; \
    buf->wt_size = 0; \
    /* try to get new block no wait, if succ, buffer is ready. */ \
    alloc = albf_alloc_buf(mgr, buf, mgr->buf_size, AIPC_NO_WAIT); \
    if (alloc == AIC_SUCCESS) \
        mgr->buffer_busy = 0; \
 \
    AOS_INTERRUPT_RESTORE \
 \
    /* before send the block, format it. */ \
    if (mgr->alfm) \
        mgr->alfm->format_data(sn, blk.addr, blk.size); \
 \
    /* try to send it no wait. */ \
    send = ablk_send_block(mgr->log_brr, mgr->link_id, &blk, AIPC_NO_WAIT); \
 \
    /* realloc a block for the buffer. */ \
    if (alloc != AIC_SUCCESS) { \
        mgr->wait_remote = 1; \
        alloc = albf_alloc_buf(mgr, buf, mgr->buf_size, AIPC_WAIT_FOREVER); \
        AIC_DUMMY_ASSERT(alloc == AIC_SUCCESS); \
        mgr->wait_remote = 0; \
        mgr->buffer_busy = 0; \
    } \
 \
    /* resend the block. */ \
    if (send != AIC_SUCCESS) { \
        /* record wait remote cnt for debug. */ \
        mgr->wait_remote = 2; \
        /* send it, wait forever. */ \
        send = ablk_send_block(mgr->log_brr, mgr->link_id, &blk, AIPC_WAIT_FOREVER); \
        AIC_DUMMY_ASSERT(send == AIC_SUCCESS); \
        mgr->wait_remote = 0; \
    } \
 \
    AOS_INTERRUPT_DISABLE \
    mgr->buffer_busy = 0; \
} while (0)

/**
 * albf_mgr_flush_boot_logs - flush boot logs.
 * if mgr->offset > 0, boot log need flush.
 * afterflush boot logs, mgr->offset reset to 0.
 */
#define albf_mgr_flush_boot_logs(mgr) \
do { \
    int index; \
    AOS_INTERRUPT_DECLARE \
 \
    for (index = 0; index < mgr->offset; index++) \
    { \
        AOS_INTERRUPT_DISABLE \
        buf = mgr->bufs + index; \
        if (buf->sn) \
            albf_flush_buf(mgr, buf); \
        AOS_INTERRUPT_RESTORE \
    } \
 \
    mgr->offset = 0; \
} while (0)
#else
static void albf_flush_buf(ALBF_MGR_T *mgr, ALOG_BUF_T *buf, u32 interrupt_save)
{ 
    u32 sn = buf->sn;
    int alloc, send;
    ABLK_T blk;

    if (buf->wt_size <= mgr->head_size) {
        mgr->read_sn = sn;
        return;
    }

    /* set the sn to 0 to indicate that it is flushed. */
    buf->sn = 0;

    /* if sn of the buffer is old than read sn, skip the buffer. */
    if (sn <= mgr->read_sn)
        return;
 
    /* set buffer busy */
    mgr->buffer_busy = 1;

    if (mgr->read_sn + 1 != sn) {
        aicx_info(0x13a2c5, "sddd", "%s: lost %d, exp sn=%d, read sn =%d",
                  mgr->name, sn - (mgr->read_sn + 1), mgr->read_sn + 1, sn);
    }

    /* updata the read sn. */
    mgr->read_sn = sn;

    /* get the flush block for send later. */
    blk.addr = (void*)buf->buf;
    blk.size = buf->wt_size; 
    buf->sn = 0;
    buf->wt_size = 0;

    /* try to get new block no wait, if succ, buffer is ready. */
    alloc = albf_alloc_buf(mgr, buf, mgr->buf_size, AIPC_NO_WAIT);
    if (alloc == AIC_SUCCESS)
        mgr->buffer_busy = 0;

    AOS_INTERRUPT_RESTORE

    /* before send the block, format it. */
    if (mgr->alfm)
        mgr->alfm->format_data(sn, blk.addr, blk.size);

    /* try to send it no wait. */ \
    send = ablk_send_block(mgr->log_brr, mgr->link_id, &blk, AIPC_NO_WAIT);

    /* realloc a block for the buffer. */
    if (alloc != AIC_SUCCESS) {
        mgr->wait_remote = 1;
        alloc = albf_alloc_buf(mgr, buf, mgr->buf_size, AIPC_WAIT_FOREVER);
        AIC_DUMMY_ASSERT(alloc == AIC_SUCCESS);
        mgr->wait_remote = 0;
        mgr->buffer_busy = 0;
    }

    /* resend the block. */
    if (send != AIC_SUCCESS) {
        /* record wait remote cnt for debug. */
        mgr->wait_remote = 2;
        /* send it, wait forever. */
        send = ablk_send_block(mgr->log_brr, mgr->link_id, &blk, AIPC_WAIT_FOREVER);
        AIC_DUMMY_ASSERT(send == AIC_SUCCESS);
        mgr->wait_remote = 0;
    }

    AOS_INTERRUPT_DISABLE
    mgr->buffer_busy = 0;
}

/**
 * albf_mgr_flush_boot_logs - flush boot logs.
 * if mgr->offset > 0, boot log need flush.
 * afterflush boot logs, mgr->offset reset to 0.
 */
void albf_mgr_flush_boot_logs(ALBF_MGR_T *mgr)
{
    ALOG_BUF_T *buf;

    int index;
    AOS_INTERRUPT_DECLARE

    for (index = 0; index < mgr->offset; index++)
    {
        AOS_INTERRUPT_DISABLE
        buf = mgr->bufs + index;
        if (buf->sn)
            albf_flush_buf(mgr, buf, interrupt_save);

        AOS_INTERRUPT_RESTORE
    }

    mgr->offset = 0;
}
#endif


/**
 * albf_mgr_flush_func - the flush thread main function, wait data and flush it.
 */
static void albf_mgr_flush_func(void *p)
{
    ALBF_MGR_T *mgr = (ALBF_MGR_T *)p;
    ALOG_BUF_T *buf;

    /* firstly, flush boot logs. */
    albf_mgr_flush_boot_logs(mgr);

    while (TRUE) {
        (void)aos_get_semaphore(mgr->sema, AIPC_WAIT_FOREVER);

        /* read and flush until empty. */
        while (mgr->read_sn != mgr->write_sn) {
            AOS_INTERRUPT_DECLARE

            AOS_INTERRUPT_DISABLE
            /* *
             * if overtake, read = cur write + 1, move to the next
             * of the write. it also is the oldest log buffer position.
             */
            if (mgr->overtake) {
                aicx_info(0x1185ef, "sdd", "%s: lost, overtake=%d, wait remote=%d\n",
                          mgr->name, mgr->overtake, mgr->wait_remote_cnt);

                mgr->overtake = 0;
                mgr->wait_remote_cnt = 0;
                mgr->rd_index = RING_NEXT_INDEX(mgr->wt_index, mgr->buf_cnt);
            }

            /* flush it, buf->sn is 0, means it has been update, do nothing. */
            buf = mgr->bufs + mgr->rd_index;
            if (buf->sn)
                albf_flush_buf(mgr, buf, interrupt_save);

            /* update rd index to next buffer index. */
            mgr->rd_index = RING_NEXT_INDEX(mgr->rd_index, mgr->buf_cnt);

            AOS_INTERRUPT_RESTORE
        }

#ifdef ALOG_FLUSH_LOG_TIMER
        /* reset the flush timer to LOG_BUF_FLUSH_TIME. */
        aos_change_timer(mgr->flush_timer, LOG_BUF_FLUSH_TIME, 0);
#endif
    }
}

#if IS_CONF_ENABLED(ALOG_TRACE_LOG_STATISTICS)
static void albf_mgr_printf_statistics(ALBF_MGR_T *mgr)
{
    u32 cur_tick, gap_tick, rate, time_us;

    mgr->stati_size += mgr->cur_buf->wt_size;
    cur_tick = alog_get_aon_timestamp();
    gap_tick = cur_tick - mgr->last_tick;
    if (gap_tick > ALOG_TRACE_STATI_ONCE_TICKS) {
        mgr->last_tick  = cur_tick;
        time_us = (gap_tick * ALOG_TRACE_STATI_ONE_TICK_US);
        if (mgr->stati_size > MAX_32BIT_DIV_1000)
            rate = ((u64)mgr->stati_size * 1000) / time_us;
        else
            rate = (mgr->stati_size * 1000) / time_us;

        if (rate) {
            aicx_info(0x0cef6e, "sdd", "%s: rate=%dbyte/ms, peak=%dbyte/ms.\n",
                      mgr->name, rate, mgr->max_rate);
            if (rate > mgr->max_rate)
                mgr->max_rate = rate;
        } else {
            rate = ((u64)mgr->stati_size * 1000000) / time_us;
            if (rate)
                aicx_info(0x00d0de, "sdd", "%s: rate=%dbyte/s, peak=%dbyte/ms.\n",
                          mgr->name, rate, mgr->max_rate);
            else
                aicx_info(0x0c6916, "sdd", "%s: size=%dbyte / time=%dus.\n",
                          mgr->name, mgr->stati_size, time_us);
        }

        mgr->stati_size = 0;
    }
}
#endif

/**
 * albf_mgr_start_flush - End a write buffer, and start the flush action.
 * It's started by a timer or it's started by call albf_mgr_get_blk_again
 * when the write current buffer does not have enough space to store logs.
 */
bool albf_mgr_start_flush(ALBF_MGR_T *mgr, boolean is_assert)
{
    u16 next_index;

    /* no log in write buffer, don't flush. */
    if (mgr->cur_buf->wt_size <= mgr->head_size)
        return FALSE;

#if IS_CONF_ENABLED(ALOG_TRACE_LOG_STATISTICS)
    if (!is_assert)
        albf_mgr_printf_statistics(mgr);
#endif

    /* calculate next_index. */
    next_index = RING_NEXT_INDEX(mgr->wt_index, mgr->buf_cnt);

    /**
     * if the next_index overtake the rd_index,
     * means no empty buffer, need to overlap old buffer.
     */
    if (next_index == mgr->rd_index) {
        mgr->overtake++;
        if (mgr->wait_remote)
            mgr->wait_remote_cnt++;

#if IS_CONF_ENABLED(ALOG_TRACE_LOST_LOG_DUMP)
        if (!is_assert) {
            if (mgr->wait_remote)
                sys_notify_ap_assert();
#if IS_CONF_ENABLED(ALOG_TRACE_LOST_LOG_NOT_WAIT_REMOTE)
            else
                AIC_ASSERT(0);
#endif
        }
#endif

        /**
         * if offset > 0, must skip it for the boot logs,
         * else move to the next index of the current read index.
         */
        if (mgr->offset) {
            if (next_index < mgr->offset)
                next_index = mgr->offset;
        }

        /*
         * if buffer_busy is 1,  means the current read buffer is not ready,
         * the write index must skip the read index.
         */
        if (mgr->buffer_busy)
            next_index = RING_NEXT_INDEX(mgr->rd_index, mgr->buf_cnt);
    }

    /* update sn to old buffer and write_sn, and sn++. */
    mgr->cur_buf->sn = mgr->sn;
    mgr->write_sn = mgr->sn;
    mgr->sn++;

    /* update write_index, and move the current writer buffer to new buffer. */
    mgr->wt_index = next_index;
    mgr->cur_buf = mgr->bufs + next_index;
    mgr->cur_buf->wt_size = 0;
    mgr->cur_buf->sn = 0;

    return (!is_assert && mgr->sema);
}

void albf_mgr_notify_flush_task(ALBF_MGR_T *mgr)
{
#ifndef WIN32
    if (!aic_slp_is_suspended() && !mgr->wait_remote)
#endif
        aos_put_semaphore(mgr->sema);
}

#ifdef ALOG_FLUSH_LOG_TIMER
/**
 * albf_mgr_flush_timer - the flush thread timerad main function.
 */
static void albf_mgr_flush_timer(void *p)
{
    ALBF_MGR_T *mgr = (ALBF_MGR_T *)p;
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    /* flush cur write buffer. */
    if (albf_mgr_start_flush(mgr, FALSE))
        albf_mgr_notify_flush_task(mgr);

    AOS_INTERRUPT_RESTORE
}
#endif

/**
 * albf_flush_buf_in_assert - Flush all the logs in the buffer after system assert.
 */
static void albf_flush_buf_in_assert(ALBF_MGR_T *mgr, ALOG_BUF_T *buf, u32 sn)
{
    ABLK_T blk;
    int send;
    boolean not_null = (AIC_IODEV_NONE != adbg_get_log_dev());

    /* not AIC_IODEV_NONE, flush all brr blk to output device, befor send. */
    if (not_null && mgr->alop)
        alop_output_flush_in_assert(mgr->alop);

    mgr->read_sn = sn;

    /* no log in write buffer, don't flush. */
    if (buf->wt_size <= mgr->head_size)
        return;

    blk.addr = (void*)buf->buf ;
    blk.size = buf->wt_size;

    /* before send it, format it. */
    if (mgr->alfm)
        mgr->alfm->format_data(sn, blk.addr, blk.size);

    /* send it to brr until success. */
    do {
        send = ablk_send_block_any(mgr->log_brr, mgr->link_id, &blk, FALSE);
        ambox_empty_recvbox_inassert();
    } while(send != AIC_SUCCESS);

    /* not AIC_IODEV_NONE, flush all brr blk to output device, after send. */
    if (not_null && mgr->alop)
        alop_output_flush_in_assert(mgr->alop);
}

static void albf_mgr_suspend(void *p)
{
#ifdef ALOG_FLUSH_LOG_TIMER
    ALBF_MGR_T *mgr = (ALBF_MGR_T *)p;

    aos_deactivate_timer(mgr->flush_timer);
#endif
}

static void albf_mgr_resume(void *p)
{
    ALBF_MGR_T *mgr = (ALBF_MGR_T *)p;
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE

    if (albf_mgr_start_flush(mgr, FALSE) && !mgr->wait_remote)
        aos_put_semaphore(mgr->sema);

    AOS_INTERRUPT_RESTORE
}

/**
 * albf_mgr_flush_buf_in_assert - flush all the buffer log after system assert.
 */
void albf_mgr_flush_all_buf_in_assert(ALBF_MGR_T *mgr)
{
    ALOG_BUF_T *buf;
    int index;
    u32 sn;

    /* flush cur write buffer. */
    albf_mgr_start_flush(mgr, TRUE);

    /* if overtake, read = cur write + 1, move to the oldest buffer. */
    if (mgr->overtake)
        mgr->rd_index = RING_NEXT_INDEX(mgr->wt_index, mgr->buf_cnt);

    /* has boot log, flush it. */
    if (mgr->offset) {
        for (index = 0; index < mgr->offset; index++)
        {
            buf = mgr->bufs + index;
            if (buf->sn) {
                sn = buf->sn;
                buf->sn = 0;
                albf_flush_buf_in_assert(mgr, buf, sn);
            }
        }
        mgr->offset = 0;
    }

    /* flush buffers until empty. */
    for (index = 0; index < mgr->buf_cnt && mgr->read_sn < mgr->write_sn; index++) {
        /* buf->sn is 0, has been update, just jump to next. */
        buf = mgr->bufs + mgr->rd_index;
        if (buf->sn) {
            sn = buf->sn;
            buf->sn = 0;
            albf_flush_buf_in_assert(mgr, buf, sn);
        }

        /* update rd index to next buffer. */
        mgr->rd_index = RING_NEXT_INDEX(mgr->rd_index, mgr->buf_cnt);
    }
}

void albf_mgr_output_block_in_assert(ALBF_MGR_T *mgr, u8 link_id, ABLK_T *blk)
{
    if (mgr->alop)
        alop_output_block_in_assert(mgr->alop, link_id, blk);
}

/**
 * albf_mgr_set_output - set the alop of the buffer mgr for output
 * the trace in buffer after system assert.
 */
void albf_mgr_set_output(ALBF_MGR_T *mgr, ALOP_T *alop)
{
    mgr->alop = alop;
}

/**
 * albf_mgr_get_blk_again -The left space of current buffer is not enough,
 * flush the current buffer and jump to the next buffer. Befor call this
 * function, the user must has call the albf_mgr_get_blk, and found left space
 * in old buffer is not enough.
 */
bool albf_mgr_get_blk_again(ALBF_MGR_T *mgr, ALBF_BLK_T *blk)
{
    ALOG_BUF_T *buf;
    bool need_flush;

    /*  flush cur write buffer. */
    need_flush = albf_mgr_start_flush(mgr, FALSE);

    /* a new buffer, reserve header size. */
    buf = mgr->cur_buf;
    buf->wt_size = mgr->head_size;
    blk->buf = buf->buf + buf->wt_size;

    /**
     * Regardless of the size of the request, blk->size set to the all buffer
     * size exclude the header. In the blk.size > buf->size - header size case,
     * just throwing away some logs.
     */
    // AIC_DUMMY_ASSERT(blk.size <= buf->size - buf->wt_size);
    blk->size = buf->size - buf->wt_size;

    return need_flush;
}

/**
 * albf_mgr_get_blk - get a block form buffer mgr, if success, must update the
 * actual used size and put it back with function albf_mgr_put_blk.
 *
 * @mgr: the instance of the buffer manager.
 * @blk: both input and output parameters.
 *       input, blk->size is the request size.
 *       output, blk->size is the block size, blk->buf is the block address.
 */
bool albf_mgr_get_blk(ALBF_MGR_T *mgr, ALBF_BLK_T *blk)
{
    ALOG_BUF_T *buf;
    bool ret = FALSE;
    buf = mgr->cur_buf;

    /* a new buffer, reserve header space. */
    if (buf->wt_size == 0) {
        /* Just put a semaphore to flush task to reset the flush timer. */
        ret = TRUE;
        buf->wt_size = mgr->head_size;
    }

    /* if left space is not enough, flush cur buf and jump to next buf */
    if (buf->wt_size + blk->size > buf->size)
        return albf_mgr_get_blk_again(mgr, blk);

    /*
     * alloc buf, here can't do below code:
     * buf->wt_size += blk.size;
     * the actual size will updata in function albf_mgr_put_blk.
     */
    blk->buf = buf->buf + buf->wt_size;
    blk->size = buf->size - buf->wt_size;

    return ret;
}

/**
 * albf_mgr_put_blk - get a blk form buffer mgr, if success, must update the
 * actual used size and put it back with function albf_mgr_put_blk.
 *
 * @mgr: the instance of the buffer manager.
 * @blk: both input parameters, blk->size is the actual used size.
 */
void albf_mgr_put_blk(ALBF_MGR_T *mgr, ALBF_BLK_T *blk)
{
    ALOG_BUF_T *buf;

    /* update the actual size. */
    buf = mgr->cur_buf;
    if (buf->wt_size +  blk->size > buf->size) {
        AIC_DUMMY_ASSERT(0);
        buf->wt_size = buf->size;
    } else {
        buf->wt_size += blk->size;
    }
}

ALBF_MGR_T *albf_mgr_create(char *buf_mgr_name, int buf_mgr_pri,
                            ABRR_HANDLE log_brr, u8 link_id,
                            const ALOG_BUF_CFG_T *buf_cfg, ALFM_T *alfm)
{
    ALBF_MGR_T *mgr;
    ALOG_BUF_T *bufs;
    u16 i, buf_cnt;
    int ret;

    aicx_debug(0x1ce5ea, "dddd", "create albf link=%d, cnt=%d, offset=%d, size=%d\n",
              link_id, buf_cfg->buf_cnt, buf_cfg->offset, buf_cfg->size);

    /* alloc buf_mgr, buf cnt must > 1. */
    buf_cnt = buf_cfg->buf_cnt;
    AIC_DUMMY_ASSERT(buf_cnt > 1);
    mgr = (ALBF_MGR_T *)aos_zalloc(sizeof(*mgr));
    AIC_DUMMY_ASSERT(mgr);

    /* alloc bufs. */
    bufs = (ALOG_BUF_T *)aos_zalloc(sizeof(*bufs) * buf_cnt);
    AIC_DUMMY_ASSERT(bufs);
    mgr->bufs = bufs;

    /* init buf_mgr. */
    mgr->alfm = alfm;
    mgr->link_id = link_id;
    mgr->log_brr = log_brr;
    mgr->buf_cnt = buf_cnt;

    /*offset must < buffer cnt. */
    AIC_DUMMY_ASSERT(buf_cfg->offset < buf_cfg->buf_cnt);
    mgr->offset = buf_cfg->offset;
    mgr->buf_size = buf_cfg->size;
    mgr->head_size = alfm->get_header_size();
    mgr->name = buf_mgr_name;
    mgr->thread =
        aos_create_thread_simply(mgr->name, albf_mgr_flush_func, mgr,
                                 LOG_BUF_STATCK_SIZE,buf_mgr_pri);
    AIC_DUMMY_ASSERT(mgr->thread != AOS_INVALID_THREAD_ID);
    mgr->sema = aos_create_semaphore_simply(mgr->name);
    AIC_DUMMY_ASSERT(mgr->sema);

#ifdef ALOG_FLUSH_LOG_TIMER
    mgr->flush_timer = aos_create_timer(mgr->name,
                                        albf_mgr_flush_timer, mgr,
                                        LOG_BUF_FLUSH_TIME,
                                        AOS_AUTO_ACTIVATE, AOS_NO_PERIOD);
    AIC_DUMMY_ASSERT(mgr->flush_timer);
#endif

    /* alloc buffers. */
    for (i = 0; i < buf_cnt; i++, bufs++) {
        bufs->sn = 0;
        bufs->wt_size = 0;
        ret = albf_alloc_buf(mgr, bufs, mgr->buf_size, AOS_NO_WAIT);
        AIC_ASSERT(ret == AIC_SUCCESS);
    }

    /* cur_buf init to the 1th buf, and the sn init to 1. */
    mgr->cur_buf = mgr->bufs + 0;
    mgr->sn = 1;

#if IS_CONF_ENABLED(ALOG_TRACE_LOG_STATISTICS)
    mgr->last_tick = alog_get_aon_timestamp();
#endif

#ifndef WIN32
    mgr->slp_handle = slp_register_sleep_hook_ex(albf_mgr_suspend, mgr,
                                              albf_mgr_resume, mgr,
                                              LV_LOW);
#endif

    return mgr;
}

void albf_mgr_destroy(ALBF_MGR_T *mgr)
{
    ALOG_BUF_T *buf;

#ifndef WIN32
    slp_unregister_sleep_hook(mgr->slp_handle);
#endif

    /* free mutex, semaphore, theread and timer. */
#ifdef ALOG_FLUSH_LOG_TIMER
    aos_deactivate_timer(mgr->flush_timer);
    aos_delete_timer(mgr->flush_timer);
#endif
    aos_delete_thread(mgr->thread);
    aos_delete_semaphore(mgr->sema);

    /* free has not been flushed buffer. */
    while (mgr->rd_index != mgr->wt_index) {
        buf = mgr->bufs + mgr->rd_index;
        if (buf->sn) {
            buf->sn = 0;
            albf_free_buf(mgr, buf);
        }
        mgr->rd_index = RING_NEXT_INDEX(mgr->rd_index, mgr->buf_cnt);
    }

    aos_free(mgr->bufs);
    aos_free(mgr);
}

