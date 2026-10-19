/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the software driver of DMA module.
*/

#include <aic_errno.h>
#include <aic_io.h>
#include <assert.h>
#include <cache.h>
#include <core_op.h>
#include <dma_cfg.h>
#include <timer.h>

#include "dma.h"
#include "dma_reg.h"

/*
 * dma_chn_desc: define dma channel description
 * @src_addr: source address
 * @wrp_str: wrap start address
 * @dst_addr: destinition address
 * @wrp_str: wrap end address
 * @xblk_len: block length and mblock length
 * @frm_len: frame length and request mode and requst status
 * @src_std_l: source transfer stride length and block stride length
 * @src_std_h: source mblock length and data size
 * @dst_std_l: destinition transfer stride length and block stride length
 * @dst_std_h: destinition mblock length and data size
 * @llp: link list pointer address
 * @misc_cfg: include link configurations and transfer type and swap mode.
 */
struct dma_chn_desc {
    u32 src_addr;
    u32 wrp_str;
    u32 dst_addr;
    u32 wrp_end;
    u32 xblk_len;
    u32 frm_len;
    u32 src_std_l;
    u32 src_std_h;
    u32 dst_std_l;
    u32 dst_std_h;
    u32 llp;
    u32 misc_cfg;
};

struct dma_chn_req {
    DMA_CHN_STATE   st;
    DMA_CALLBACK    func;
    void            *param;
    DMA_HWID        hwid;
};
static struct dma_chn_req g_dma_chn_req[DMA_CHN_MAX];

static u32 dma_base;

void aic_dma_phy_chn_enable(u32 chn)
{
    AIC_REG_SETVAL(dma_base + DMAC_CHAN_COMCFG(chn), BIT_DMAC_CHAN_EN);
}

void aic_dma_phy_chn_disable(u32 chn)
{
    AIC_WRITE32(dma_base + DMAC_CHAN_COMCFG(chn), 0x0);
}

void aic_dma_phy_chn_int_cfg(u32 chn, DMA_INT_TYPE type)
{
    AIC_WRITE32(dma_base + DMAC_CHAN_INTEN(chn), type | DMA_ERR_INT);
}

void aic_dma_phy_chn_ll_enable(u32 chn)
{
    AIC_REG_SETVAL(dma_base + DMAC_CHAN_COMCFG(chn), BIT_DMAC_CHAN_LLT_EN);
}

void aic_dma_phy_chn_set_hwid(u32 chn, DMA_HWID id)
{
    if (id) {
        AIC_WRITE32(dma_base + DMAC_CHAN_REQ(chn), BIT_DMAC_CHAN_REQSTS);
        AIC_WRITE32(dma_base + DMAC_REQ_CFG(id - 1), chn + 1);
        g_dma_chn_req[chn].hwid = id;
    }
}

void aic_dma_phy_chn_unset_hwid(u32 chn, DMA_HWID id)
{
    if (id) {
        AIC_WRITE32(dma_base + DMAC_CHAN_REQ(chn), 0x0);
        AIC_WRITE32(dma_base + DMAC_REQ_CFG(id - 1), 0x0);
        g_dma_chn_req[chn].hwid = 0;
    }
}

void aic_dma_phy_resume(void)
{
    AIC_WRITE32(dma_base + DMAC_SUSPEND, 0x0);
}

int aic_dma_phy_suspend(void)
{
    u32 val;

    AIC_WRITE32(dma_base + DMAC_SUSPEND, BIT_DMAC_SUSPEND_REQ);

    do {
        val = AIC_READ32(dma_base + DMAC_SUSPEND);
        val &= BIT_DMAC_SUSPEND_STS;
    } while (!val);

    return AIC_SUCCESS;
}

void aic_dma_phy_chn_resume(u32 chn)
{
    AIC_WRITE32(dma_base + DMAC_CHAN_SUSPEND(chn), 0);
}

int aic_dma_phy_chn_suspend(u32 chn)
{
    u32 val;

    AIC_WRITE32(dma_base + DMAC_CHAN_SUSPEND(chn), BIT_DMAC_CHAN_SUSPEND);

    do {
        val = AIC_READ32(dma_base + DMAC_CHAN_SUSPEND(chn));
        val &= BIT_DMAC_CHAN_SUSPEND_STS;
    } while (!val);

    return AIC_SUCCESS;
}

void aic_dma_phy_chn_polling_done(u32 chn)
{
    u32 status;

    do {
        status = AIC_READ32(dma_base + DMAC_CHAN_RAWINTSTS(chn));
    } while (!(status & BIT_DMAC_CHAN_FRAME_DONE_RAWINT));

    AIC_WRITE32(dma_base + DMAC_CHAN_INTCLR(chn), 0xF);
}

int aic_dma_phy_chn_polling_timeout(u32 chn, u32 timeout_ms)
{
    u32 status, cnt_old, cnt_cur, polling_ms;
    int ret = AIC_SUCCESS;

    cnt_old = aic_get_timestamp();

    do {
        status = AIC_READ32(dma_base + DMAC_CHAN_RAWINTSTS(chn));
        cnt_cur = aic_get_timestamp();
        polling_ms = cnt_cur - cnt_old;
        if (polling_ms >= timeout_ms) {
            ret = AIC_ETIMEDOUT;
            break;
        }
    } while (!(status & BIT_DMAC_CHAN_FRAME_DONE_RAWINT));

    AIC_WRITE32(dma_base + DMAC_CHAN_INTCLR(chn), 0xF);

    return ret;
}

u32 aic_dma_phy_chn_get_src(u32 chn)
{
    return AIC_READ32(dma_base + DMAC_SAR_0(chn));
}

u32 aic_dma_phy_chn_get_dst(u32 chn)
{
    return AIC_READ32(dma_base + DMAC_DAR_0(chn));
}

void aic_dma_phy_chn_start(u32 chn)
{
    AIC_WRITE32(dma_base + DMAC_CHAN_REQ(chn), BIT_DMAC_CHAN_SW_REQ);
}

static void aic_dma_phy_cfg_illegal_check(struct dma_chn_cfg *cfg)
{
    ASSERT(cfg->blk_len <= DMAC_BLOCK_LEN_MASK);
    ASSERT(cfg->mblk_len <= DMAC_MBLOCK_LEN_MASK);
    ASSERT(cfg->frame_len <= DMAC_FRAME_LEN_MASK);
    ASSERT((cfg->src_trsf_std <= DMAC_TRANS_STRIDE_MASK) && (cfg->dst_trsf_std <= DMAC_TRANS_STRIDE_MASK));
    ASSERT((cfg->src_blk_std <= DMAC_BLOCK_STRIDE_MASK) && (cfg->dst_blk_std <= DMAC_BLOCK_STRIDE_MASK));
    ASSERT((cfg->src_mblk_std <= DMAC_MBLOCK_STRIDE_MASK) && (cfg->dst_mblk_std <= DMAC_MBLOCK_STRIDE_MASK));
    ASSERT(cfg->hwid < DMA_REQ_HWID_MAX);
    ASSERT((cfg->src_ds <= DMA_DWORD_SIZE) && (cfg->dst_ds <= DMA_DWORD_SIZE));
    ASSERT(!(cfg->src_addr%(1<<cfg->src_ds)));
    ASSERT(!(cfg->dst_addr%(1<<cfg->dst_ds)));
    ASSERT(cfg->trsf_type <= DMA_DEV_TO_DEV);
    ASSERT(cfg->wrap <= DMA_DST_WRAP);
    ASSERT(cfg->swap <= DMA_BADC);
    ASSERT(cfg->req <= DMA_LINK_REQ);
    ASSERT(cfg->int_type <= DMA_ERR_INT);
}

int aic_dma_phy_chn_cfg(u32 chn, struct dma_chn_cfg *cfg)
{
    struct dma_chn_desc *chn_desc;
    u32 frm_reg, src_std_h, dst_std_h, misc;
    u32 src_type, dst_type;

    aic_dma_phy_cfg_illegal_check(cfg);

    chn_desc = (struct dma_chn_desc *)(dma_base + DMAC_CHAN_DESC_REG_OFFSET + 0x30 * chn);

    aic_dma_phy_chn_int_cfg(chn, cfg->int_type);
    aic_dma_phy_chn_set_hwid(chn, cfg->hwid);

    /* linklist mode need only cfg ll enable and llp */
    if (cfg->llp) {
        aic_dma_phy_chn_ll_enable(chn);
        chn_desc->llp = cfg->llp;
        return AIC_SUCCESS;
    }

    switch (cfg->trsf_type) {
    case DMA_MEM_TO_MEM:
        src_type = 1;
        dst_type = 1;
        break;
    case DMA_MEM_TO_DEV:
        src_type = 1;
        dst_type = 0;
        break;
    case DMA_DEV_TO_MEM:
        src_type = 0;
        dst_type = 1;
        break;
    case DMA_DEV_TO_DEV:
        src_type = 0;
        dst_type = 0;
        break;
    default:
        src_type = 1;
        dst_type = 1;
        break;
    }

    switch (cfg->wrap) {
    case DMA_SRC_WRAP:
        src_type = 2;
        break;
    case DMA_DST_WRAP:
        dst_type = 2;
        break;
    default:
        break;
    }

    /* combin reg cfg */
    frm_reg = BIT_DMAC_REQ_MODE(cfg->req) | cfg->frame_len;
    src_std_h = BIT_DMAC_SRC_DATA_SIZE(cfg->src_ds) | cfg->src_mblk_std;
    dst_std_h = BIT_DMAC_DST_DATA_SIZE(cfg->dst_ds) | cfg->dst_mblk_std;
    misc = BIT_DMAC_SWAP_MODE(cfg->swap) |
            BIT_DMAC_DST_TRANS_TYPE(dst_type) |
            BIT_DMAC_SRC_TRANS_TYPE(src_type) |
            cfg->int_type << BIT_DMAC_LLT_INT_OFFSET |
            cfg->end << BIT_DMAC_LLT_END_OFFSET;

    if (!cfg->src_trsf_std) cfg->src_trsf_std = 1 << cfg->src_ds;
    if (!cfg->dst_trsf_std) cfg->dst_trsf_std = 1 << cfg->dst_ds;

    chn_desc->src_addr      = cfg->src_addr;
    chn_desc->dst_addr      = cfg->dst_addr;
    chn_desc->wrp_str       = cfg->wrp_str;
    chn_desc->wrp_end       = cfg->wrp_end;
    chn_desc->xblk_len      = BIT_DMAC_MBLK_LENGTH(cfg->mblk_len) | cfg->blk_len;
    chn_desc->frm_len       = frm_reg;
    chn_desc->src_std_l     = BIT_DMAC_SRC_BLK_STRIDE(cfg->src_blk_std) | cfg->src_trsf_std;
    chn_desc->src_std_h     = src_std_h;
    chn_desc->dst_std_l     = BIT_DMAC_DST_BLK_STRIDE(cfg->dst_blk_std) | cfg->dst_trsf_std;
    chn_desc->dst_std_h     = dst_std_h;
    chn_desc->misc_cfg      = misc;
    __DSB();

    return AIC_SUCCESS;
}

int aic_dma_phy_ll_cfg(u32 chn, u32 llp, struct dma_chn_cfg *cfg, u32 node_cnt)
{
    struct dma_chn_desc *chn_desc;
    u32 frm_reg, src_std_h, dst_std_h, misc;
    u32 i, src_type, dst_type;

    for (i = 0; i < node_cnt; i++) {
        switch (cfg[i].trsf_type) {
        case DMA_MEM_TO_MEM:
            src_type = 1;
            dst_type = 1;
            break;
        case DMA_MEM_TO_DEV:
            src_type = 1;
            dst_type = 0;
            break;
        case DMA_DEV_TO_MEM:
            src_type = 0;
            dst_type = 1;
            break;
        case DMA_DEV_TO_DEV:
            src_type = 0;
            dst_type = 0;
            break;
        default:
            src_type = 1;
            dst_type = 1;
            break;
        }

        switch (cfg->wrap) {
        case DMA_SRC_WRAP:
            src_type = 2;
            break;
        case DMA_DST_WRAP:
            dst_type = 2;
            break;
        default:
            break;
        }

        /* combin reg cfg */
        frm_reg = BIT_DMAC_REQ_MODE(cfg[i].req) | cfg[i].frame_len;
        src_std_h = BIT_DMAC_SRC_DATA_SIZE(cfg[i].src_ds) | cfg[i].src_mblk_std;
        dst_std_h = BIT_DMAC_DST_DATA_SIZE(cfg[i].dst_ds) | cfg[i].dst_mblk_std;
        misc = BIT_DMAC_SWAP_MODE(cfg[i].swap) |
                BIT_DMAC_DST_TRANS_TYPE(dst_type) |
                BIT_DMAC_SRC_TRANS_TYPE(src_type) |
                cfg[i].int_type << BIT_DMAC_LLT_INT_OFFSET;

        if (!cfg[i].src_trsf_std) cfg[i].src_trsf_std = 1 << cfg[i].src_ds;
        if (!cfg[i].dst_trsf_std) cfg[i].dst_trsf_std = 1 << cfg[i].dst_ds;

        chn_desc = (struct dma_chn_desc *)(llp + 0x30 * i);
        chn_desc->src_addr      = cfg[i].src_addr;
        chn_desc->dst_addr      = cfg[i].dst_addr;
        chn_desc->wrp_str       = cfg[i].wrp_str;
        chn_desc->wrp_end       = cfg[i].wrp_end;
        chn_desc->xblk_len      = BIT_DMAC_MBLK_LENGTH(cfg[i].mblk_len) | cfg[i].blk_len;
        chn_desc->frm_len       = frm_reg;
        chn_desc->src_std_l     = BIT_DMAC_SRC_BLK_STRIDE(cfg[i].src_blk_std) | cfg[i].src_trsf_std;
        chn_desc->src_std_h     = src_std_h;
        chn_desc->dst_std_l     = BIT_DMAC_DST_BLK_STRIDE(cfg[i].dst_blk_std) | cfg[i].dst_trsf_std;
        chn_desc->dst_std_h     = dst_std_h;
        chn_desc->llp           = (u32)(chn_desc) + DMAC_CHAN_DESC_REG_SIZE;
        chn_desc->misc_cfg      = misc;
        if (i == node_cnt - 1) {
            if (cfg[i].end == DMA_LLP_ENDED) {
                chn_desc->misc_cfg |= BIT_DMAC_LLT_END;
                chn_desc->llp = 0;
            } else {
                /* linklist endless mode */
                chn_desc->llp = llp;
            }
        }
    }

    /* clean linklist node memory */
    dcache_clean_by_range(llp, sizeof(struct dma_chn_desc) * node_cnt);

    aic_dma_phy_chn_cfg(chn, cfg);

    return AIC_SUCCESS;
}

void aic_dma_phy_common_init(void)
{
    AIC_WRITE32(dma_base + DMAC_INTEN, BIT_DMAC_INTEN);
    //TODO, add more init configurations
}

u32 aic_dma_phy_get_int(void)
{
    return AIC_READ32(dma_base + DMAC_CHANINT_MASKSTS);
}

void aic_dma_phy_clr_int(u32 chn)
{
    AIC_WRITE32(dma_base + DMAC_CHAN_INTCLR(chn),
                    BIT_DMAC_CHAN_ERROR_INTCLR |
                    BIT_DMAC_CHAN_LLT_DONE_INTCLR |
                    BIT_DMAC_CHAN_FRAME_DONE_INTCLR |
                    BIT_DMAC_CHAN_MBLK_DONE_INTCLR |
                    BIT_DMAC_CHAN_BLK_DONE_INTCLR);
}

u32 aic_dma_phy_chn_int_status(u32 chn)
{
    return AIC_READ32(dma_base + DMAC_CHAN_MASKINTSTS(chn));
}

void aic_dma_phy_module_request(u32 index)
{
    dma_base = dma_bd_cfg[index].base;
}

