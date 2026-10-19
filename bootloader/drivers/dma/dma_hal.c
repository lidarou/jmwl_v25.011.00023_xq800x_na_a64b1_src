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
#include <console/console.h>
#include <initcall.h>
#include <dma_cfg.h>

#include "dma.h"
#include "dma_interface.h"

struct dma_chn_req {
    DMA_CHN_STATE   st;
    DMA_CALLBACK    func;
    void            *param;
    DMA_HWID        hwid;
};
struct dma_chn_req g_dma_chn_req[DMA_CHN_MAX];

#define DMA_CHN_CHK(chn) do {\
        if ((chn) >= DMA_CHN_MAX)\
            ASSERT("DMA: it is illegal dma channel number!\n");\
    } while(0)

/*
 * aic_dma_resume() - Resume the whole dma ip.
 * It could be used in low power follow.
 */
void aic_dma_resume(void)
{
    aic_dma_phy_resume();
}

/*
 * aic_dma_suspend() - Suspend the whole dma ip.
 * It could be used in low power follow.
 */
int aic_dma_suspend(void)
{
    return aic_dma_phy_suspend();
}

/*
 * aic_dma_chn_resume() - Suspend the channel of dma ip.
 * @chn: the resume dma channel.
 */
void aic_dma_chn_resume(u32 chn)
{
    DMA_CHN_CHK(chn);

    aic_dma_phy_chn_resume(chn);
}

/*
 * aic_dma_chn_suspend() - Suspend the channel of dma ip.
 * @chn: the resume dma channel.
 */
int aic_dma_chn_suspend(u32 chn)
{
    DMA_CHN_CHK(chn);

    return aic_dma_phy_chn_suspend(chn);
}

/*
 * aic_dma_chn_polling_done() - Polling the channel transfer done.
 * @chn: the dma polling channel.
 */
void aic_dma_chn_polling_done(u32 chn)
{
    aic_dma_phy_chn_polling_done(chn);
}

/*
 * aic_dma_chn_polling_timeout() - Polling the channel transfer until timeout.
 * @chn: the dma polling channel.
 * @timeout: timeout param
 */
int aic_dma_chn_polling_timeout(u32 chn, u32 timeout_ms)
{
    return aic_dma_phy_chn_polling_timeout(chn, timeout_ms);
}

/*
 * aic_dma_chn_get_src() - Get the source address of this dma channel.
 * @chn: the dma channel need to be read.
 * Return this channel's source address.
 */
u32 aic_dma_chn_get_src(u32 chn)
{
    DMA_CHN_CHK(chn);

    return aic_dma_phy_chn_get_src(chn);
}

/*
 * aic_dma_chn_get_dst() - Get the destination address of this dma channel.
 * @chn: the dma channel need to be read.
 * Return this channel's destination address.
 */
u32 aic_dma_chn_get_dst(u32 chn)
{
    DMA_CHN_CHK(chn);

    return aic_dma_phy_chn_get_dst(chn);
}

/*
 * aic_dma_chn_start() - make this dma channel start to transfer.
 * @chn: the start dma channel.
 * Notice, if it is hw request channel, it will not start immediately,
 * it will be started by the hwid master ip.
 */
void aic_dma_chn_start(u32 chn)
{
    DMA_CHN_CHK(chn);

    aic_dma_phy_chn_enable(chn);

    if (!g_dma_chn_req[chn].hwid)
        aic_dma_phy_chn_start(chn);
}

/*
 * aic_dma_chn_cfg() - Normal configure with the dma channel.
 * @chn: the configured dma channel.
 * @cfg: the consumer dma configurations.
 */
int aic_dma_chn_cfg(u32 chn, struct dma_chn_cfg *cfg)
{
    DMA_CHN_CHK(chn);

    g_dma_chn_req[chn].hwid = cfg->hwid;

    return aic_dma_phy_chn_cfg(chn, cfg);
}

/*
 * aic_dma_ll_cfg() - Linklist configure with the dma channel.
 * @chn: the configured dma channel.
 * @llp: the linklist physical address, it should be allocted by consumer.
 * @cfg: the consumer dma configurations.
 * @node_cnt: indicate how many linklist nodes it needs to configure.
 */
int aic_dma_ll_cfg(u32 chn, u32 llp, struct dma_chn_cfg *cfg, u32 node_cnt)
{
    DMA_CHN_CHK(chn);

    /* linklist pointer must be 32bit align */
    ASSERT(IS_ALIGNED(llp, 0x20));

    g_dma_chn_req[chn].hwid = cfg->hwid;

    return aic_dma_phy_ll_cfg(chn, llp, cfg, node_cnt);
}

/*
 * aic_dma_chn_request() - Request a dma channel.
 * @cb: If this dma channel transfer done(with interrupt), this callback will be called.
 * @param: the callback input parameter.
 * Return dma channel number if success or return AIC_ENODEV.
 */
int aic_dma_chn_request(DMA_CALLBACK cb, void *param)
{
    u32 chn;

    AIC_INT_SAVE_AREA

    AIC_INT_DISABLE
    for (chn = 0; chn < DMA_CHN_MAX; chn++) {
        if (!g_dma_chn_req[chn].st) {
            g_dma_chn_req[chn].st    = DMA_CHN_BUSY;
            g_dma_chn_req[chn].func  = cb;
            g_dma_chn_req[chn].param = param;
            g_dma_chn_req[chn].hwid  = 0;
            break;
        }
    }

    /* judgement if it is the first chn, if yes enable clock and init. */
    if (!chn) {
        /* set dma glb */
        AIC_REG_SETVAL(dma_bd_cfg[SYS_DMA].glb_base, dma_bd_cfg[SYS_DMA].glb_bit);
        aic_dma_phy_common_init();
    }
    AIC_INT_RESTORE

    if (chn >= DMA_CHN_MAX) {
        log_print("DMA request chn failed, donot have enough channel!");
        return AIC_ENODEV;
    }
    return chn;
}

/*
 * aic_dma_chn_free() - Free a dma channel.
 * @chn: the free dma channel.
 */
void aic_dma_chn_free(u32 chn)
{
    AIC_INT_SAVE_AREA

    DMA_CHN_CHK(chn);

    AIC_INT_DISABLE
    if (!g_dma_chn_req[chn].st) {
        log_print("dma channel %d had free already!", chn);
        AIC_INT_RESTORE
        return;
    }

    if (g_dma_chn_req[chn].hwid)
        aic_dma_phy_chn_unset_hwid(chn, g_dma_chn_req[chn].hwid);

    aic_dma_phy_clr_int(chn);
    aic_dma_phy_chn_disable(chn);

    g_dma_chn_req[chn].func  = NULL;
    g_dma_chn_req[chn].param = NULL;
    g_dma_chn_req[chn].hwid  = 0;
    g_dma_chn_req[chn].st    = DMA_CHN_IDLE;

    /* check if all the dma chn are idle, if yes disable clock */
    for (chn = 0; chn < DMA_CHN_MAX; chn++) {
        if (g_dma_chn_req[chn].st != DMA_CHN_IDLE) {
            AIC_INT_RESTORE
            return;
        }
    }

    /* clr dma glb */
    AIC_REG_CLRVAL(dma_bd_cfg[SYS_DMA].glb_base, dma_bd_cfg[SYS_DMA].glb_bit);

    AIC_INT_RESTORE
}

int aic_dma_init(void)
{
    aic_dma_phy_module_request(SYS_DMA);

    return AIC_SUCCESS;
}
early_init_late(aic_dma_init);

