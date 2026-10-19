/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#include <aic_errno.h>
#include <usb/usb_device.h>
#include <usb/usb_dma.h>
#include <usb/usb_dma_regs.h>
#include <io.h>
#include <cache.h>

#define DBG_DMA_BASIC_MSG           0x00000100
#define DBG_DMA_VERBOSE_MSG         0x00000200
#define DBG_DMA_ERR_MSG             0x00000400
#define DBG_DMA_CHAIN_MSG           0x00000800
#define DBG_DMA_CHANNEL_USEGE_MSG   0x00001000
#define DBG_DMA_IRQ_MSG             0x00002000

uint32_t divRoundUp(uint32_t divident, uint32_t divisor) {
    return divisor ? ((divident + divisor -1) / divisor) : 0;
}

void usb_dma_FreeTrbChain(DmaController * pD, DmaTrbChainDesc * trbChainDesc)
{

    uint32_t i =0;

    USB_PRINTF("Free %p TRBs pool start %d end %d numOfTrbs %d\n",
            trbChainDesc,
            trbChainDesc->start,
            trbChainDesc->end,
            trbChainDesc->numOfTrbs);

    USB_PRINTF(
            " mapSize %d channel %p trbAddrStart %p trbDmaAddrStart %08x allocated\n",
            trbChainDesc->mapSize,
            trbChainDesc->channel,
            trbChainDesc->trbPool,
            trbChainDesc->trbDMAAddr);


    for(i= 0; i< trbChainDesc->mapSize; i++ )
        pD->trbChainFreeMap[(trbChainDesc->start>>3)+i]= 0x00;
}

static uint32_t usb_dma_AllocTrbChain(DmaController *pD, DmaChannel *channel,
        uint32_t numOfTrbs)
{
    DmaTrbChainDesc *trbChainDesc = &channel->trbChainDesc;
    uint32_t i, j, count=0, mapCount=0;

    USB_PRINTF("TRB_MAP_SIZE %d, addr %p selected\n",
            TRB_MAP_SIZE, trbChainDesc);

    /** Two additional TRB for LINK TRb and bad TRB, and in order to simplify
     * algorittm firmware round number of TRBs to module 8*/
    mapCount = (numOfTrbs+2+7) >> 3;
    /*Firmware round up reserved number of TRBS to modulo 8*/
    for(i=0; i< TRB_MAP_SIZE; i++) {

        if(pD->trbChainFreeMap[i] == 0x0) {
            count++;
        } else
            count =0;

        if(count == mapCount) {
            break;
        }
    }

    if(count != mapCount) {
        USB_PRINTF("No free TRBs%s\n", "");
        return AIC_ENOMEM;
    }

    trbChainDesc->reserved =1;
    trbChainDesc->channel = channel;
    trbChainDesc->actualLen = 0;
    trbChainDesc->mapSize = mapCount;
    trbChainDesc->numOfTrbs  = numOfTrbs;
    trbChainDesc->start= (i+1 - mapCount) << 3;
    trbChainDesc->end= trbChainDesc->start;
    trbChainDesc->trbPool = ((DmaTrb*)pD->trbPoolAddr) + trbChainDesc->start;
    trbChainDesc->trbDMAAddr = (uint32_t)(uintptr_t)((DmaTrb*)pD->trbDMAPoolAddr+ trbChainDesc->start);

    USB_PRINTF("start %d end %d nTrbs %d"
            " mSize %d chan %p trbArS %p tDmaS %08x alloc\n",
            trbChainDesc->start,
            trbChainDesc->end,
            trbChainDesc->numOfTrbs,
            trbChainDesc->mapSize,
            trbChainDesc->channel,
            trbChainDesc->trbPool,
            trbChainDesc->trbDMAAddr);

    for(j= 0; j< count; j++ ) {
        pD->trbChainFreeMap[i-j]= 0xFF;
    }

    return 0;
}

static void sgdmShowTrbChain(DmaTrbChainDesc * trbChainDesc) {
    uint16_t i = 0;

    USB_PRINTF( "TRB Chain %p for channel %p\n",
            trbChainDesc, trbChainDesc->channel);

    USB_PRINTF("idx    | trb size | trb addr | "
            "FLAG: NORMAL LINK CHAIN  ALL %s\n", " ");

    for (i = 0; i < trbChainDesc->numOfTrbs+2; i++) {
        USB_PRINTF("%02d     | %08x | %08x |"
                "         %d      %d     %d    %08x\n",
                i,
                trbChainDesc->trbPool[i].dmaSize & TD_SIZE_MASK,
                trbChainDesc->trbPool[i].dmaAddr,
                (trbChainDesc->trbPool[i].ctrl & TD_TYPE_NORMAL) ? 1 : 0,
                (trbChainDesc->trbPool[i].ctrl & TD_TYPE_LINK) ? 1 : 0,
                (trbChainDesc->trbPool[i].ctrl & TDF_CHAIN_BIT) ? 1 : 0,
                trbChainDesc->trbPool[i].ctrl);
    }
}

/******************************************************************************
 * Private Driver functions for AIC DMA
 *****************************************************************************/
/**
 * Function fill in TRBs in trbChainDesc
 */
static uint32_t usb_dma_NewTdHw(DmaController * pD, DmaTrbChainDesc * trbChainDesc)
{
    uint32_t i = 0;
    uint32_t tmp = 0;

    uint32_t startAddress = trbChainDesc->dwStartAddress;
    DmaChannel *channel = trbChainDesc->channel;
    uint32_t dataSize = channel->maxTrbLen;

    if (trbChainDesc->numOfTrbs > 1) {
        for (i = 0; i < (trbChainDesc->numOfTrbs - 1); i++) {
            if (channel->dmultEnabled) {
                BUILD_NORMAL_TRB_NO_IOC(trbChainDesc->trbPool[i],
                                            startAddress, dataSize, 0);
            } else {
                BUILD_NORMAL_TRB_NO_IOC_CHAIN(trbChainDesc->trbPool[i],
                                                    startAddress, dataSize, 0);
            }

            startAddress += dataSize;
            tmp += dataSize;
        }
    }

    BUILD_NORMAL_TRB(trbChainDesc->trbPool[i], startAddress, trbChainDesc->len - tmp, 0);

    if (channel->dmultEnabled) {
            BUILD_EMPTY_TRB(trbChainDesc->trbPool[i+1]);
    }

    sgdmShowTrbChain(trbChainDesc);
    dcache_invcc_by_range((int32_t)trbChainDesc->trbPool,
        sizeof(DmaTrb) * (trbChainDesc->numOfTrbs + 2));

    return AIC_SUCCESS;
}

/**
 * Function starts DMA transfer
 */
static void usb_dma_ArmTd(DmaController * pD,
                            DmaTrbChainDesc * trbChainDesc)
{
    DmaChannel * channel = trbChainDesc->channel;
    uint32_t cfg =0;

    writel(&pD->regs->ep_sel, (uint32_t) (channel->isDirTx |
        channel->hwUsbEppNum));

    USB_PRINTF("Set DORBEL ChNr-chain-Dir-EpNr-traddr: 0x%p-0x%p-%s-%d-0x%x\n",
        channel, trbChainDesc, (channel->isDirTx) ? "Tx" : "Rx",
        channel->hwUsbEppNum, trbChainDesc->trbDMAAddr);

    writel(&pD->regs->traddr, (uint32_t) trbChainDesc->trbDMAAddr);
	writel(&pD->regs->ep_sts, DMARF_EP_TRBERR);
    writel(&pD->regs->ep_cmd, DMARF_EP_DRDY);

    cfg = readl(&pD->regs->ep_cfg);
    if(!(cfg & DMARV_EP_ENABLED)) {
        cfg |= DMARV_EP_ENABLED;
        writel(&pD->regs->ep_cfg, (uint32_t)cfg);
    }
}

/**
 * Function free trbChainDesc and inform upper layer that transfer has been finished
 */
static uint32_t
usb_dma_UpdateDescBuffer(DmaController * pD,
                       DmaTrbChainDesc * trbChainDesc,
                       uint16_t status)
{
    DmaChannel * channel= trbChainDesc->channel;
    uint32_t size, i;

    for (i = 0; i < trbChainDesc->numOfTrbs; i++) {
        size = ((uint32_t)trbChainDesc->trbPool[i].dmaSize) & TD_SIZE_MASK;
        trbChainDesc->actualLen += size;
    }

    /*This value can by read in complete callback function*/
    channel->lastTransferLength = trbChainDesc->actualLen;
    USB_PRINTF("Complete callback channel-actualLen %p-%d\n",
            channel, trbChainDesc->actualLen);
    channel->status = CUSBDMA_STATUS_FREE;

    if (pD->callback.complete) {
        pD->callback.complete(channel->hwUsbEppNum,
                channel->isDirTx);
    }
    usb_dma_FreeTrbChain(pD, trbChainDesc);
    return AIC_SUCCESS;
}

uint32_t usb_dma_init(DmaController * ctrl,
                    const CUSBDMA_Config* config,
                    CUSBDMA_Callbacks * callbacks)
{
    if (!ctrl || !config || !callbacks)
        return AIC_EINVAL;

    if(!config->trbAddr || !config->trbDmaAddr)
        return AIC_EINVAL;

    USB_PRINTF("Initialization AIC DMA component\n");
    memset((void*) ctrl, 0, sizeof(DmaController));
    memset((void*)config->trbAddr, 0, TRB_POOL_SIZE);

    ctrl->trbDMAPoolAddr = config->trbDmaAddr;
    ctrl->trbPoolAddr = config->trbAddr;

    ctrl->regs = (DmaRegs*) config->regBase;
    ctrl->cfg = *config;
    ctrl->callback = *callbacks;
    return AIC_SUCCESS;
}

int usb_dma_start(DmaController * ctrl)
{
    uint8_t i =0;
    if (!ctrl)
        return AIC_EINVAL;

    USB_PRINTF("Starting AIC DMA component\n");
    ctrl->dmaMode = DMA_MODE_CHANNEL_INDIVIDUAL;
    if ((ctrl->cfg.dmaModeRx & ctrl->cfg.dmaModeTx) == 0xFFFF) {
        writel(&ctrl->regs->conf, DMARF_DMULT);
        ctrl->dmaMode = DMA_MODE_GLOBAL_DMULT;
    } else if ((ctrl->cfg.dmaModeRx | ctrl->cfg.dmaModeTx) == 0x0000) {
        writel(&ctrl->regs->conf, DMARF_DSING);
        ctrl->dmaMode = DMA_MODE_GLOBAL_DSINGL;
    }

    for(i=0; i< MAX_DMA_CHANNELS; i++) {
        if(ctrl->cfg.dmaModeRx & (1 << i)) {
            ctrl->rx[i].dmultEnabled =1;
            ctrl->rx[i].maxTrbLen = TD_DMULT_MAX_TRB_DATA_SIZE;
            ctrl->rx[i].maxTdLen = TD_DMULT_MAX_TD_DATA_SIZE;
        } else {
            ctrl->rx[i].dmultEnabled =0;
            ctrl->rx[i].maxTrbLen = TD_SING_MAX_TRB_DATA_SIZE;
            ctrl->rx[i].maxTdLen = TD_SING_MAX_TD_DATA_SIZE;
        }

        ctrl->rx[i].lastTransferLength =0;

        if(ctrl->cfg.dmaModeTx & (1 << i)) {
            ctrl->tx[i].dmultEnabled =1;
            ctrl->tx[i].maxTrbLen = TD_DMULT_MAX_TRB_DATA_SIZE;
            ctrl->tx[i].maxTdLen = TD_DMULT_MAX_TD_DATA_SIZE;
        } else {
            ctrl->tx[i].dmultEnabled =0;
            ctrl->tx[i].maxTrbLen = TD_SING_MAX_TRB_DATA_SIZE;
            ctrl->tx[i].maxTdLen = TD_SING_MAX_TD_DATA_SIZE;
        }
        ctrl->tx[i].lastTransferLength =0;
    }

    return 0;
}

void * usb_dma_channel_alloc(DmaController *ctrl, uint8_t isDirTx,
                                uint8_t hwEpNum, uint8_t isIsoc)
{
    DmaRegs * regs = NULL;
    DmaChannel * channel = NULL;
    uint32_t dma_ien;
    uint32_t cfg = 0;
    uint16_t dmaMode = 0;

    if ( hwEpNum >= MAX_DMA_CHANNELS)
        return NULL;

    regs = ctrl->regs;
    dma_ien = readl(&regs->ep_ien);
    if (isDirTx) {
        if (ctrl->tx[hwEpNum].status >= CUSBDMA_STATUS_FREE) {
            USB_PRINTF("Channel 0x%p dedicated for Ep %d is already reserved\n",
                    &ctrl->tx[hwEpNum], hwEpNum);
            return NULL;
        }
        channel = &ctrl->tx[hwEpNum];
        channel->isDirTx = 0x80;
        dma_ien |= (0x01 << (hwEpNum + 16));
        dmaMode = ctrl->cfg.dmaModeTx;
    } else {
        if (ctrl->rx[hwEpNum].status >= CUSBDMA_STATUS_FREE) {
            USB_PRINTF("Channel 0x%p dedicated for Ep %d is already reserved\n",
                    &ctrl->rx[hwEpNum], hwEpNum);
            return NULL;
        }
        channel = &ctrl->rx[hwEpNum];
        channel->isDirTx = 0x00;
        dma_ien |= (0x01 << hwEpNum);
        dmaMode = ctrl->cfg.dmaModeRx;
    }

    USB_PRINTF("Channel 0x%p has been allocated for ep %d\n", channel, hwEpNum);
    channel->numOfTrbChain=0;
    channel->controller = ctrl;
    channel->dmultGuard = 0;
    channel->status = CUSBDMA_STATUS_FREE;
    channel->hwUsbEppNum = hwEpNum;

    writel(&regs->ep_sel, (uint32_t) hwEpNum | channel->isDirTx);
    cfg = readl(&regs->ep_cfg);

    if (ctrl->dmaMode == DMA_MODE_CHANNEL_INDIVIDUAL) {
        if (dmaMode & (1 << hwEpNum)) {
            cfg |= DMARV_EP_DMULT;
        } else {
            cfg |= DMARV_EP_DSING;
        }
    }

    writel(&regs->ep_sts, DMARF_EP_IOC | DMARF_EP_ISP |
            DMARF_EP_TRBERR /*| DMARF_EP_ISOERR*/);
    writel(&regs->ep_cfg, (uint32_t) DMARV_EP_ENABLED | cfg);
    writel(&regs->ep_ien, dma_ien);
    writel(&regs->ep_sts_en,
 			DMARF_EP_TRBERR /*| DMARF_EP_ISOERR| DMARF_EP_DESCMIS | DMARF_EP_OUTSMM*/);

    return channel;
}

uint32_t usb_dma_channel_abort(DmaController * ctrl, void * channel)
{
    DmaChannel * dmaChannel = (DmaChannel*) channel;
    uint32_t cfg=0;

    if (!ctrl || !channel)
        return AIC_EINVAL;

    USB_PRINTF("Channel 0x%p has been aborted\n", channel);
    /*Disable DMA*/
    writel(&ctrl->regs->ep_sel,
            (uint32_t) (dmaChannel->isDirTx | dmaChannel->hwUsbEppNum));
    cfg = readl(&ctrl->regs->ep_cfg);
    cfg &= ~DMARV_EP_ENABLED;
    writel(&ctrl->regs->ep_cfg, (uint32_t)cfg);
    /*Clear all interrupts for this channel*/
    writel(&ctrl->regs->ep_sts, 0xFFFFFFFF);

    usb_dma_FreeTrbChain(ctrl, &dmaChannel->trbChainDesc);

    if(dmaChannel->status != CUSBDMA_STATUS_UNKNOW)
        dmaChannel->status = CUSBDMA_STATUS_FREE;
    return AIC_SUCCESS;
}

uint32_t usb_dma_channel_release(DmaController *ctrl, void *channel)
{
    DmaChannel * dmaChannel = (DmaChannel *) channel;
    uint32_t dmaIen;

    USB_PRINTF("Dma channel release ch: %p\n", channel);
    if (!channel || !ctrl)
        return AIC_EINVAL;

    /*Disable endpoint and interrupt for DMA. Probably it should be in dmaChannelAbort*/
    dmaIen = readl(&ctrl->regs->ep_ien);
    if (dmaChannel->isDirTx) {
        dmaIen &= ~(0x01 << (dmaChannel->hwUsbEppNum + 16));
    } else {
        dmaIen &= ~(0x01 << dmaChannel->hwUsbEppNum);
    }

    writel(&ctrl->regs->ep_sel,
            (uint32_t) dmaChannel->hwUsbEppNum | dmaChannel->isDirTx);
    writel(&ctrl->regs->ep_cfg, (uint32_t) 0);

    writel(&ctrl->regs->ep_ien, dmaIen);
    writel(&ctrl->regs->ep_sts_en, 0x0);
    writel(&ctrl->regs->ep_cmd, DMARF_EP_EPRST);

    writel(&ctrl->regs->ep_sts, DMARF_EP_IOC | DMARF_EP_ISP |
            DMARF_EP_TRBERR | DMARF_EP_ISOERR);

    if(dmaChannel->status >= CUSBDMA_STATUS_BUSY)
        dmaChannel->status = CUSBDMA_STATUS_ABORT;

    usb_dma_channel_abort(ctrl, channel);

    USB_PRINTF("Channel 0x%p has been released\n", channel);

    dmaChannel->status = CUSBDMA_STATUS_UNKNOW;
    return AIC_SUCCESS;
}

uint32_t
usb_dma_channel_program(DmaController * ctrl, void * channel,
            uint16_t packetSize, uintptr_t dmaAddr, uint32_t len)
{
    DmaTrbChainDesc *trbChainDesc;
    DmaChannel * dmaChannel;
    uint8_t retval;
    uint32_t numOfTrbs;

    if (!channel)
        return AIC_EINVAL;

    dmaChannel = (DmaChannel*) channel;
    if (dmaChannel->status != CUSBDMA_STATUS_FREE)
        return AIC_EINVAL;
    USB_PRINTF(
        "DMA prepare for channel: 0x%p ep%d-%s packetSize %d, dmaAddr 0x%lx length %d\n",
    dmaChannel, dmaChannel->hwUsbEppNum, dmaChannel->isDirTx ? "Tx" : "Rx",
    packetSize, dmaAddr, len);

    numOfTrbs = divRoundUp(len, dmaChannel->maxTrbLen);
    retval = usb_dma_AllocTrbChain(ctrl, channel, numOfTrbs);
    if(retval)
        return retval;

    trbChainDesc = &dmaChannel->trbChainDesc;
    trbChainDesc->dwStartAddress = dmaAddr;
    trbChainDesc->len = len;
    dmaChannel->wMaxPacketSize = packetSize;

    usb_dma_NewTdHw(ctrl, trbChainDesc);
    dmaChannel->lastTransferLength = 0;
    usb_dma_ArmTd(ctrl, trbChainDesc);

    dmaChannel->status = CUSBDMA_STATUS_BUSY;

    return AIC_SUCCESS;
}

/*
 * this function should be called when system indicates interrupt for DMA.
 */
void usb_dma_isr(DmaController *ctrl)
{
    DmaRegs * regs = NULL;
    uint32_t epSts, epIsts, cfg;
    DmaChannel *channel = NULL;
    DmaTrbChainDesc *trbChainDesc = NULL;
    int i;

    if (!ctrl)
        return;

    regs = ctrl->regs;

    // Check interrupts for all endpoints
    epIsts = readl(&regs->ep_ists);
    if (epIsts == 0)
        return;

    USB_PRINTF("DMAIRQ ep_ists SFR: %x\n", epIsts);
    for (i = 0; i < 32; i++) {
        uint8_t isDirTx;
        uint8_t epNum;

        if (!(epIsts & (1 << i)))
            continue;

        isDirTx = i > 15 ? DMARD_EP_TX : 0u;
        epNum = isDirTx ? i - 16 : i;
        if (epNum >= MAX_DMA_CHANNELS)
            return;

        writel(&regs->ep_sel, epNum | isDirTx);

        epSts = readl(&regs->ep_sts);

        if (isDirTx) /*transmit data*/{
            channel = &ctrl->tx[epNum];
        } else {
            channel = &ctrl->rx[epNum];
        }

        USB_PRINTF("DMA IRQ epSts %08x epNum: %d epDir: %s\n",
                epSts, epNum, (isDirTx ? "TX" : "RX"));

        if (epSts & DMARF_EP_TRBERR) {
            USB_PRINTF("DMAIRQ: TRBERR %s\n", "");
            writel(&regs->ep_sts, DMARF_EP_TRBERR);
        } else {
            cfg = readl(&regs->ep_cfg);
            cfg &= ~DMARV_EP_ENABLED;
            writel(&regs->ep_cfg, (uint32_t)cfg);
        }

        if ((epSts & DMARF_EP_IOC) || (epSts & DMARF_EP_ISP)
                || (channel->dmultGuard & DMARF_EP_IOC)
                || (channel->dmultGuard & DMARF_EP_ISP)) {
            trbChainDesc = &channel->trbChainDesc;
            writel(&regs->ep_sts, DMARF_EP_IOC | DMARF_EP_ISP);
            channel->dmultGuard = 0;
            usb_dma_UpdateDescBuffer(ctrl, trbChainDesc, 0);
        }

        if (epSts & DMARF_EP_OUTSMM) {
            writel(&regs->ep_sts, DMARF_EP_OUTSMM);
            USB_PRINTF("DMAIRQ: OUTSMM%s\n", "");
        }

        if (epSts & DMARF_EP_DESCMIS) {
            writel(&regs->ep_sts, DMARF_EP_DESCMIS);
            USB_PRINTF("DMAIRQ: DESCMIS%s\n", "");
        }
    }
}

CUSBDMA_Status
usb_dma_get_channel_status(DmaController *ctrl, void *channel)
{
    DmaChannel * ch = (DmaChannel*)channel;

    if (!ctrl || !channel) {
        while (1);
        return AIC_EINVAL;
    }

    /* If DMA has been armed and has not finished yet then
     * update status to *_ARMED. This procedure allows to detect
     * that DMA HW is in progress.
     */
    if(ch->status >= CUSBDMA_STATUS_BUSY) {
        uint32_t dorbel, status;
        writel(&ctrl->regs->ep_sel,
            (uint32_t) (ch->isDirTx | ch->hwUsbEppNum));

        dorbel = readl(&ctrl->regs->ep_cmd);
        status = readl(&ctrl->regs->ep_sts);

        if((dorbel & DMARF_EP_DRDY) || (status & DMARF_EP_DBUSY)) {
            ch->status = CUSBDMA_STATUS_ARMED;
        } else {
            ch->status = CUSBDMA_STATUS_BUSY;
        }
    }

   return ch->status;
}

uint32_t usb_dma_get_actual_length(void *pD, void *channel)
{
    DmaChannel * ch = (DmaChannel*) channel;

    if (!pD || !ch)
        return AIC_EINVAL;

    return ch->lastTransferLength;
}

uint32_t usb_dma_set_max_length(void *pD, void *channel, uint32_t size)
{
    DmaChannel * ch = (DmaChannel*) channel;

    if (!pD || !ch)
        return AIC_EINVAL;

    if(ch->dmultEnabled) {
        ch->maxTrbLen = size;
    } else {
        ch->maxTrbLen  = (size > TD_SING_MAX_TRB_DATA_SIZE) ?
            TD_SING_MAX_TRB_DATA_SIZE : size;
    }
    return AIC_SUCCESS;
}

uint32_t usb_dma_get_max_length(void *pD, void *channel)
{
    DmaChannel * ch = (DmaChannel*) channel;
    if (!pD || !ch)
        return AIC_EINVAL;

    return ch->maxTdLen;
}
