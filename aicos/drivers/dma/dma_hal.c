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

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x093432
#define LOG_MODULE    AMOD_DMA

#include <aic_auto_reg.h>
#include <aic_assert.h>
#include <aic_dma_chn.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_interrupt.h>
#include <aic_io.h>
#include <aic_osa.h>
#include <cache.h>
#include <core_op.h>
#include <dma_cfg.h>
#include <irq.h>

#include "dma.h"
#include "dma_interface.h"

/*
 * dma_chn_desc: define dma channel description
 * @src_addr: source address
 * @wrp_str: wrap start address
 * @dst_addr: destinition address
 * @wrp_end: wrap end address
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
struct dma_chn_req g_dma_chn_req[DMA_CHN_MAX];

#define DMA_CHN_CHK(chn) do {\
        if ((chn) >= DMA_CHN_MAX)\
            AIC_ASSERT("DMA illegal chn\n");\
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
    AIC_ASSERT(IS_ALIGNED(llp, 0x20));

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
        aicx_error(0x02445a, "", "DMA chn full");
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
        aicx_error(0x0c9447, "d", "dma chn %d freed err", chn);
        AIC_INT_RESTORE
        return;
    }

    if (g_dma_chn_req[chn].hwid)
        aic_dma_phy_chn_unset_hwid(chn, g_dma_chn_req[chn].hwid);

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

/*
 * aic_dma_bare_cpy
 * It will always use dma chn0 to copy memory.
 * Notice!!!
 * It can be only used when boot up or resume, normal mode cannot use
 * this api.
 */
void aic_dma_bare_cpy(u32 dst, u32 src, u32 size)
{
    aic_dma_phy_bare_cpy(dst, src, size);
}

ISR_RET_E aic_dma_irq_handler(void *val)
{
    DMA_CALLBACK chn_cb;
    void *chn_param;
    u32 chn, status = aic_dma_phy_get_int();

    if (!status) {
        aicx_error(0x119728, "", "DMA int status error");
        return AIC_EPERM;
    }

    for (chn = 0; status != 0; status &= ~(1 << chn)) {
        chn = aic_find_first_bit(status);
        DMA_CHN_CHK(chn);
        aic_dma_phy_clr_int(chn);

        chn_cb = g_dma_chn_req[chn].func;
        chn_param = g_dma_chn_req[chn].param;
        if (chn_cb == NULL) {
            aicx_error(0x142021, "", "dma irq callback error");
            return AIC_EPERM;
        }
        if (chn_param != NULL)
            chn_cb(chn_param);
        else
            chn_cb((void *)chn);
    }

    return NONE_WORKER;
}

int aic_dma_init(void)
{
    u32 dma_max = sizeof(dma_bd_cfg) / sizeof(*dma_bd_cfg);
    u32 i;

    for (i = 0; i < dma_max; i++) {
        register_irq(dma_bd_cfg[i].irq_no, aic_dma_irq_handler, &dma_bd_cfg[i], NULL, NULL);
    }

    aic_dma_phy_module_request(SYS_DMA);

    return AIC_SUCCESS;
}
