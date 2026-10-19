/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file make statment of dma
 *
 */

#ifndef __DMA_H__
#define __DMA_H__

#include <aic_type.h>
#include <aic_dma_chn.h>

#define DMA_BLOCK_LEN_MAX   0xFFFF
#define DMA_MBLOCK_LEN_MAX  0xFFFF
#define DMA_FRAME_LEN_MAX   0xFFFF

typedef void (*DMA_CALLBACK)(void *param);

typedef enum {
    DMA_CHN_IDLE = 0,
    DMA_CHN_BUSY
} DMA_CHN_STATE;

typedef enum {
    DMA_NONE_TRIG = 0,
    DMA_HW_TRIG,
    DMA_SW_TRIG
} DMA_TRIG_MODE;

typedef enum {
    DMA_BLOCK_REQ = 0,
    DMA_MBLOCK_REQ,
    DMA_FRAME_REQ,
    DMA_LINK_REQ
} DMA_REQ_MODE;

typedef enum {
    DMA_NONE_INT = 0,
    DMA_BLK_INT,
    DMA_MBLK_INT,
    DMA_MBLK_BLK_INT,
    DMA_FRM_INT,
    DMA_FRM_BLK_INT,
    DMA_FRM_MBLK_INT,
    DMA_FRM_MBLK_BLK_INT,
    DMA_LL_INT,
    DMA_LL_BLK_INT,
    DMA_LL_MBLK_INT,
    DMA_LL_MBLK_BLK_INT,
    DMA_LL_FRM_INT,
    DMA_LL_FRM_BLK_INT,
    DMA_LL_FRM_MBLK_INT,
    DMA_LL_FRM_MBLK_BLK_INT,
    DMA_ERR_INT
} DMA_INT_TYPE;

typedef enum {
    DMA_MEM_TO_MEM = 0,
    DMA_MEM_TO_DEV,
    DMA_DEV_TO_MEM,
    DMA_DEV_TO_DEV
} DMA_TRSF_TYPE;

typedef enum {
    DMA_NONE_WRAP = 0,
    DMA_SRC_WRAP,
    DMA_DST_WRAP
} DMA_WRAP_MODE;

typedef enum {
    DMA_BYTE_SIZE = 0,
    DMA_HWORD_SIZE,
    DMA_WORD_SIZE,
    DMA_DWORD_SIZE
} DMA_DATA_SIZE;

typedef enum {
    DMA_ABCD = 0,
    DMA_DCBA,
    DMA_CDAB,
    DMA_BADC
} DMA_SWAP_MODE;

typedef enum {
    DMA_LLP_ENDED = 0,
    DMA_LLP_CONTINUE
} DMA_LLP_END;

typedef enum {
    SYS_DMA = 0,
    AON_DMA
} DMA_TYPE;

/*
 * dma_chn_cfg: define dma channel configurations
 * @src_addr: source address
 * Notice, source address should align with 1<<src_ds
 * @dst_addr: destinition address
 * Notice, source address should align with 1<<dst_ds
 * @wrp_str: wrap start address
 * @wrp_end: wrap end address
 * @llp: link list address
 * Notice, link list should be physical address, and allocted by consumer
 * @blk_len: block length, the minimun transfer unit
 * Notice, it should be less then 0x10000
 * @mblk_len: mulitpule block length, it should be muli block lenght
 * Notice, it should be less then 0x10000
 * @frame_len: frame length, the total transfer length
 * Notice, it should be less then 0X10000000
 * @src_trsf_std: source transfer stride length
 * @src_blk_std: source block stride length
 * @src_mblk_std: source mblock stride length
 * @dst_trsf_std: destinition transfer stride length
 * @dst_blk_std: destinition block stride length
 * @dst_mblk_std: destinition mblock stride length
 * @hwid: dma hardware request id
 * Notice, DMA_REQ_SOFTWARE will only be used for software request
 * @src_ds: source data size
 * @dst_ds: destinition data size
 * @trsf_type: dma transfer type
 * @wrap: dma transfer wrap mode
 * @swap: dma transfer swap mode
 * @req: dma transfer request mode, dma transfer corresponding length per request
 * @int_type: the corresponding lenght transfer done trigger dma interrupt
 * @end: the link list end flag, if it is ENDED, this will be the last link node
 * @reserved: reserved one word for 32bit align
 */
struct dma_chn_cfg {
    u32 src_addr;
    u32 dst_addr;
    u32 wrp_str;
    u32 wrp_end;
    u32 llp;
    u32 blk_len;
    u32 mblk_len;
    u32 frame_len;
    u32 src_trsf_std;
    u32 src_blk_std;
    u32 src_mblk_std;
    u32 dst_trsf_std;
    u32 dst_blk_std;
    u32 dst_mblk_std;
    DMA_HWID hwid;
    DMA_DATA_SIZE src_ds;
    DMA_DATA_SIZE dst_ds;
    DMA_TRSF_TYPE trsf_type;
    DMA_WRAP_MODE wrap;
    DMA_SWAP_MODE swap;
    DMA_REQ_MODE req;
    DMA_INT_TYPE int_type;
    DMA_LLP_END end;
    u32 reserved;
};

/*
 * aic_dma_resume() - Resume the whole dma ip.
 * It could be used in low power follow.
 */
void aic_dma_resume(void);
/*
 * aic_dma_suspend() - Suspend the whole dma ip.
 * It could be used in low power follow.
 */
int aic_dma_suspend(void);
/*
 * aic_dma_chn_resume() - Suspend the channel of dma ip.
 * @chn: the resume dma channel.
 */
int aic_dma_chn_suspend(u32 chn);
/*
 * aic_dma_chn_suspend() - Suspend the channel of dma ip.
 * @chn: the resume dma channel.
 */
void aic_dma_chn_resume(u32 chn);
/*
 * aic_dma_chn_polling_done() - Polling the channel transfer done.
 * @chn: the dma polling channel.
 */
void aic_dma_chn_polling_done(u32 chn);
/*
 * aic_dma_chn_polling_timeout() - Polling the channel transfer until timeout.
 * @chn: the dma polling channel.
 * @timeout: timeout param
 */
int aic_dma_chn_polling_timeout(u32 chn, u32 timeout_ms);
/*
 * aic_dma_chn_get_src() - Get the source address of this dma channel.
 * @chn: the dma channel need to be read.
 * Return this channel's source address.
 */
u32 aic_dma_chn_get_src(u32 chn);
/*
 * aic_dma_chn_get_dst() - Get the destination address of this dma channel.
 * @chn: the dma channel need to be read.
 * Return this channel's destination address.
 */
u32 aic_dma_chn_get_dst(u32 chn);
/*
 * aic_dma_chn_start() - make this dma channel start to transfer.
 * @chn: the start dma channel.
 * Notice, if it is hw request channel, it will not start immediately,
 * it will be started by the hwid master ip.
 */
void aic_dma_chn_start(u32 chn);
/*
 * aic_dma_chn_cfg() - Normal configure with the dma channel.
 * @chn: the configured dma channel.
 * @cfg: the consumer dma configurations.
 */
int aic_dma_chn_cfg(u32 chn, struct dma_chn_cfg *cfg);
/*
 * aic_dma_ll_cfg() - Linklist configure with the dma channel.
 * @chn: the configured dma channel.
 * @llp: the linklist physical address, it should be allocted by consumer.
 * @cfg: the consumer dma configurations.
 * @node_cnt: indicate how many linklist nodes it needs to configure.
*/
int aic_dma_ll_cfg(u32 chn, u32 llp, struct dma_chn_cfg *cfg, u32 node_cnt);
u32 aic_dma_chn_int_status(u32 chn);
/*
 * aic_dma_chn_request() - Request a dma channel.
 * @cb: If this dma channel transfer done(with interrupt), this callback will be called.
 * @param: the callback input parameter.
 * Return dma channel number if success or return AIC_ENODEV.
 */
int aic_dma_chn_request(DMA_CALLBACK cb, void *param);
/*
 * aic_dma_chn_free() - Free a dma channel.
 * @chn: the free dma channel.
 */
void aic_dma_chn_free(u32 chn);
/*
 * aic_dma_memcpy() - Normal dma memory copy api.
 * @src: dma memcpy source address.
 * @dst: dma memcpy destination address.
 * @src: dma memcpy data size.
 */
void aic_dma_memcpy(u32 src, u32 dst, u32 size);
void aic_dma_module_request(u32 index);

int aic_dma_init(void);

#endif /* __DMA_H__ */

