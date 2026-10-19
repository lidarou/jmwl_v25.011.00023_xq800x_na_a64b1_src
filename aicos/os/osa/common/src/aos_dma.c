
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x165900
#define LOG_MODULE    AMOD_AOS
#include <aic_assert.h>
#include <aic_osa.h>
#include <aic_size.h>
#include <aos_thread.h>
#include <dma.h>

#define FAST_BLK_SIZE    0x8000
#define MAX_FRM_SIZE    (SIZE_256M - 1)

void aos_dma_cpy_callback(void *param)
{
    aos_put_semaphore((aos_semaphore_ptr) param);
}

void aos_dma_cpy(void *dst, void *src, size_t size)
{
    struct dma_chn_cfg dma_cfg = {0};
    aos_semaphore_ptr done_sem;
    DMA_DATA_SIZE ds;
    u32 blk_len;
    int chn;

    AIC_ASSERT(size < MAX_FRM_SIZE);

    /* create a sempore. */
    done_sem = aos_create_semaphore_simply("dma_cpy");
    AIC_ASSERT(done_sem);

    do {
        /* request a dma channel. */
        chn = aic_dma_chn_request(aos_dma_cpy_callback, done_sem);
        /* failed, try again later. */
        if (chn < 0)
            aos_sleep(10);
    } while (chn < 0);

    ds = ((u32)dst%4 || (u32)src%4) ? DMA_BYTE_SIZE : DMA_WORD_SIZE;
    blk_len = (size > DMA_BLOCK_LEN_MAX) ? FAST_BLK_SIZE : size;

    dma_cfg.src_addr   = (u32)src;
    dma_cfg.dst_addr   = (u32)dst;
    dma_cfg.blk_len    = blk_len;
    dma_cfg.mblk_len   = blk_len;
    dma_cfg.frame_len  = size;
    dma_cfg.src_ds     = ds;
    dma_cfg.dst_ds     = ds;
    dma_cfg.trsf_type  = DMA_MEM_TO_MEM;
    dma_cfg.int_type   = DMA_FRM_INT;
    dma_cfg.req        = DMA_FRAME_REQ;

    /* start dma copy. */
    aic_dma_chn_cfg(chn, &dma_cfg);
    aic_dma_chn_start(chn);

    /* wait complete. */
    aos_get_semaphore(done_sem, AOS_WAIT_FOREVER);

    /* free channel and semphore. */
    aic_dma_chn_free(chn);
    aos_delete_semaphore(done_sem);
}

void aos_dma_cpy_raw(void *dst, void *src, size_t size)
{
    struct dma_chn_cfg dma_cfg = {0};
    DMA_DATA_SIZE ds;
    u32 blk_len;
    u32 chn;

    AIC_ASSERT(size < MAX_FRM_SIZE);

    do {
        /* request a dma channel. */
        chn = aic_dma_chn_request(AIC_NULL, AIC_NULL);
    } while (chn < 0);

    ds = ((u32)dst%4 || (u32)src%4) ? DMA_BYTE_SIZE : DMA_WORD_SIZE;
    blk_len = (size > DMA_BLOCK_LEN_MAX) ? FAST_BLK_SIZE : size;

    dma_cfg.src_addr   = (u32)src;
    dma_cfg.dst_addr   = (u32)dst;
    dma_cfg.blk_len    = blk_len;
    dma_cfg.mblk_len   = blk_len;
    dma_cfg.frame_len  = size;
    dma_cfg.src_ds     = ds;
    dma_cfg.dst_ds     = ds;
    dma_cfg.trsf_type  = DMA_MEM_TO_MEM;
    dma_cfg.int_type   = DMA_NONE_INT;
    dma_cfg.req        = DMA_FRAME_REQ;
    dma_cfg.hwid       = DMA_REQ_SOFTWARE;

    /* start dma copy. */
    aic_dma_chn_cfg(chn, &dma_cfg);
    aic_dma_chn_start(chn);
    aic_dma_chn_polling_done(chn);
    aic_dma_chn_free(chn);
}

/*
 * aos_dma_bare_cpy
 * It will always use dma chn0 to copy memory.
 * Notice!!!
 * It can be only used when boot up or resume, normal mode cannot use
 * this api.
 */
void aos_dma_bare_cpy(void *dst, void *src, size_t size)
{
    aic_dma_bare_cpy((u32)dst, (u32)src, size);
}

