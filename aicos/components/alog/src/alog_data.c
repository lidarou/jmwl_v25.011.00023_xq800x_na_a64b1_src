/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The data log source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0eaa6a
#define LOG_MODULE    AMOD_ALOG

#include <aic_osa.h>
#include <alog_inner.h>
#include <alog_data.h>
#include <alog_data_format.h>
#include <alog_frame.h>
#ifndef WIN32
#include <cache.h>
#endif
#include <stdio.h>

#define ALOG_DATA_NAME "lgdt"

static ALOG_DATA_T *s_alog_data;

#define MIN_DMA_COPY_SIZE       0x80
#define DSPLOG_DEBUG            0

#define MAX_DUMP_NAME_SIZE      12
#define ADDR_SIZE               (sizeof(u32) * 2)
#define NAME_ADDR_SIZE          (MAX_DUMP_NAME_SIZE + ADDR_SIZE)

#if IS_CONF_ENABLED(USE_DMA_CPY)

static void alog_data_cpy(void *dst, void *src, size_t size)
{
    if (!aic_sys_is_assert() && size > MIN_DMA_COPY_SIZE) {
        /* flush source area.make sure get the latest data */
        dcache_clean_by_range(DCLINE_ALIGN(src), DCLINE_GRANULARITY(src, size));
        /* clean and invalid the dst area to avoid cache overlay the dma copy data. */
        dcache_invcc_by_range(DCLINE_ALIGN(dst), DCLINE_GRANULARITY(dst, size));
        aos_dma_cpy(dst, src, size);
        return;
    }
}
#else
#define alog_data_cpy(dst, src, size)    memcpy(dst, src, size)
#endif

void alog_data_ex(u8 *data, size_t size, u8 type, u8 link, int timeout)
{
    size_t left = size, once_write, head_size, write = 0, data_size;
    ALOG_DATA_T *alog_data = s_alog_data;
    bool is_assert = aic_sys_is_assert();
    ABRR_HANDLE brr;
    ABLK_T blk;
    int ret;

    aicx_debug(0x0d38fa, "xxdd", "alog_data_ex, base=0x%x, size=0x%x, type=%d, link=%d.\n",
              data, size, type, link);

    if (!alog_data)
        return;

    brr = alog_data->brr;
    once_write = ablk_get_max_pool_size(brr);
    head_size = alog_data_get_header_size(type);

    while (left) {
        once_write = MIN(once_write, left + head_size);
        blk.size = once_write;

        if (is_assert) {
            /* assert dump, poll until success. */
            while(ablk_get_block_any(brr, once_write, &blk, FALSE))
                ;
        } else {
            ret = ablk_get_block(brr, once_write, &blk, timeout);
            if (ret) {
                aicx_debug(0x0828e1, "d", "data lost, get blk ret=%d.\n", ret);
                alog_data->lost_cnt++;
                return;
            }
        }
        data_size = once_write - head_size;
        /* firstly, dma copy data exclusive head. */
        alog_data_cpy((u8*)blk.addr + head_size, data + write, data_size);

        /* than, format the head. */
        if (ALOG_DATA_DUMP == type)
            alog_data_format_ex(link, type, INVALID_DATA_LINK, blk.addr, once_write, FALSE);
        else
            alog_data_format(type, link, blk.addr, once_write, FALSE);

        if (is_assert) {
#ifdef CONFIG_SUBSYS_AP
            alog_output_data_in_assert(blk.addr, blk.size);
            ablk_recycle_block_any(brr, &blk, FALSE);
#else
            /* assert dump, poll until success. */
            while(ablk_send_block_any(brr, ALDATA_LINK, &blk, FALSE))
                ;
#endif
        } else {
            ret = ablk_send_block(brr, ALDATA_LINK, &blk, timeout);
            if (ret) {
                aicx_debug(0x13f4df, "d", "data lost, send blk ret=%d.\n", ret);
                alog_data->lost_cnt++;
                ablk_recycle_block(brr, &blk);
                return;
            }
        }

        /* exclusive head. */
        write += data_size;
        left -= data_size;
    }
}

void alog_dump_start(u8 sys, char *name, u32 addr, u32 size)
{
    size_t data_size = ALOG_FRAME_H_SIZE;
    ALOG_FRAME_H_T *frame;
    ABRR_HANDLE brr;
    ABLK_T blk;
    u32 *data;
    int ret;

    if (!s_alog_data)
        return;

    brr = s_alog_data->brr;
    data_size += NAME_ADDR_SIZE;

    ret = ablk_get_block(brr, data_size, &blk, AIPC_WAIT_FOREVER);
    AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);
    frame = (ALOG_FRAME_H_T *)blk.addr;

    DATA_SET_FRAME_HEAD_EX(frame, sys, 0, data_size, LOG_PRO_DATA_DUMP,
                           0, INVALID_DATA_LINK, SUB_PRO_START);

    data = (u32*)(frame + 1);
    *data++ = addr;
    *data++ = size;

    snprintf((void *)data, MAX_DUMP_NAME_SIZE, "%s", name);
    ret = ablk_send_block(brr, ALDATA_LINK, &blk, AIPC_WAIT_FOREVER);
    AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);
}

/**
 * alog_data_finish - will finish a link dump.
 *
 * @type: the type of the data.
 * @link: the virtual link id of the data.(ALOG_DATA_DUMP, link is the sys)
 */
void alog_data_finish(u8 type, u8 link)
{
    u8 prot = alog_data_type_2_prot(type);
    ALOG_FRAME_H_T *frame;
    u8 sys = AIC_THIS_SYS;
    u8 link_id = link;
    ABRR_HANDLE brr;
    ABLK_T blk;
    int ret;

    aicx_debug(0x04fcf7, "dd", "alog_data_finish, type=%d, link=%d.\n", type, link);

    if (!s_alog_data)
        return;

    brr = s_alog_data->brr;
    ret = ablk_get_block(brr, ALOG_FRAME_H_SIZE, &blk, AIPC_WAIT_FOREVER);
    AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);
    frame = (ALOG_FRAME_H_T *)blk.addr;

    if (ALOG_DATA_DUMP == type) {
        sys = link;
        link_id = INVALID_DATA_LINK;
    }

    DATA_SET_FRAME_HEAD_EX(frame, sys, 0, ALOG_FRAME_H_SIZE, prot,
                           0, link_id, SUB_PRO_FINISH);

    ret = ablk_send_block(brr, ALDATA_LINK, &blk, AIPC_WAIT_FOREVER);
    AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);
}

#ifdef CONFIG_SUPPORT_ADSP
struct adsp_trace_header {
    u16 trace_len;
#if ALOG_ADSP_AIDX
    u8  b_idx;
    u8  resrve;
#endif
};

u32 s_adsp_sn;
#define ADSP_TRACE_STATCK_SIZE 768

void alog_dsp_trace(u8 *buf, size_t size)
{
    struct adsp_trace_header *head = (struct adsp_trace_header *)buf;
    size_t left, max_size, head_size, once, write = 0;
    ALOG_DATA_T *alog_data = s_alog_data;
    u8 *data = (u8 *)(head + 1);
    ABRR_HANDLE brr;
    ABLK_T blk;
    int ret;
    bool b_idx = FALSE;

#if ALOG_ADSP_AIDX
    b_idx = head->b_idx;
#endif

    ++s_adsp_sn;
    AIC_ASSERT(head->trace_len < size);
    brr = alog_data->brr;
    max_size = ablk_get_max_pool_size(brr);
    head_size = alog_adsp_get_header_size();
    left = head->trace_len + head_size;
    head->trace_len = 0;

    once = MIN(max_size, left);
    ret = ablk_get_block(brr, once, &blk, 0);
    if (ret) {
        aicx_debug(0x0f933e, "d", "dsp log lost, get blk ret=%d.\n", ret);
        return;
    }

    /* firstly, copy data exclusive head. */
    alog_data_cpy((u8*)blk.addr + head_size, data + write, once - head_size);
    /* than, format the head. */
    alog_adsp_log_format(blk.addr, left, s_adsp_sn, b_idx);

    ret = ablk_send_block(brr, ALDATA_LINK, &blk, 0);
    if (ret) {
        aicx_debug(0x0e8d9a, "d", "dsp log lost, send blk ret=%d.\n", ret);
        ablk_recycle_block(brr, &blk);
        return;
    }
    left -= once;
    write += (once - head_size);

    while (left) {
        once = MIN(max_size, left);
        ret = ablk_get_block(brr, once, &blk, 0);
        if (ret) {
            aicx_debug(0x0f933e, "d", "dsp log lost, get blk ret=%d.\n", ret);
            return;
        }
        alog_data_cpy((u8*)blk.addr, data + write, once);
        ret = ablk_send_block(brr, ALDATA_LINK, &blk, 0);
        if (ret) {
            aicx_debug(0x0e8d9a, "d", "dsp log lost, send blk ret=%d.\n", ret);
            ablk_recycle_block(brr, &blk);
            return;
        }
        left -= once;
        write += once;
    }
}
#endif

ALOG_DATA_T *alog_data_create(u8 sys)
{
    ALOG_DATA_T *alog_data;

    alog_data = aos_malloc(sizeof(*alog_data));
    AIC_DUMMY_ASSERT(alog_data);

    alog_data->brr = ablk_request_brr(sys, AIPC_BRR_DEBUG);
    alog_data->lost_cnt = 0;

    s_alog_data = alog_data;

    return alog_data;
}

void alog_data_destroy(ALOG_DATA_T *alog_data)
{
    if (!alog_data)
        return;

    s_alog_data = NULL;
    ablk_release_brr(alog_data->brr);
    aos_free(alog_data);
}

