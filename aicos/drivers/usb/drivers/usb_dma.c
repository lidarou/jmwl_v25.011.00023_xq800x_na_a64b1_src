/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0530af
#define LOG_MODULE    AMOD_USB

#include <aic_errno.h>
#include <aic_io.h>
#include <cache.h>
#include <core_op.h>
#include <timer_inc.h>
#include <usb_device.h>
#include <usb_dma.h>
#include <usb_dma_regs.h>

#define DBG_DMA_BASIC_MSG                   0x00000100
#define DBG_DMA_VERBOSE_MSG                 0x00000200
#define DBG_DMA_ERR_MSG                     0x00000400
#define DBG_DMA_CHAIN_MSG                   0x00000800
#define DBG_DMA_CHANNEL_USEGE_MSG           0x00001000
#define DBG_DMA_IRQ_MSG                     0x00002000

/******************************************************************************
 * Private Driver functions and macros - list management
 *****************************************************************************/
static inline DmaTrbChainDesc *
usb_dma_ListGetTrbChainDescEntry(ListHead * list)
{
    return (DmaTrbChainDesc *) ((uintptr_t) list
            - (uintptr_t) &(((DmaTrbChainDesc *) 0)->chainNode));
}

uint32_t divRoundUp(uint32_t divident, uint32_t divisor)
{
    return divisor ? ((divident + divisor -1) / divisor) : 0;
}

void usb_dma_FreeTrbChain(DmaController * pD,
                          DmaTrbChainDesc * trbChainDesc)
{
    uint32_t i =0;

    USB_AIDXPF(0x0b00c5, "pddd", "Free %p TRBs pool start %d end %d numOfTrbs %d\n",
            trbChainDesc,
            trbChainDesc->start,
            trbChainDesc->end,
            trbChainDesc->numOfTrbs);

    USB_AIDXPF(0x1b41f6, "dppx", " mapSize %d channel %p trbAddrStart %p trbDmaAddrStart %08x allocated\n",
            trbChainDesc->mapSize,
            trbChainDesc->channel,
            trbChainDesc->trbPool,
            trbChainDesc->trbDMAAddr);


    for(i= 0; i< trbChainDesc->mapSize; i++ )
        pD->trbChainFreeMap[(trbChainDesc->start>>3)+i]= 0x00;

    trbChainDesc->channel->numOfTrbChain--;
    trbChainDesc->reserved =0;
    listDelete(&trbChainDesc->chainNode);

}

static uint32_t
usb_dma_AllocTrbChain(DmaController *pD, DmaChannel *channel,
                      uint32_t numOfTrbs, DmaTrbChainDesc ** chain)
{
    uint32_t i=0, j, count=0, mapCount=0;
    DmaTrbChainDesc * trbChainDesc = NULL;
    /* find free descriptors for TRB chain */
    for(i=0; i< TAB_SIZE_OF_DMA_CHAIN; i++) {
        if(!pD->trbChainDesc[i].reserved) {
            trbChainDesc = &pD->trbChainDesc[i];
            break;
        }
    }

    *chain = NULL;
    if(!trbChainDesc) {
        USB_AIDXPF(0x1320d5, "s", "No free TRB Chain Descriptor%s\n", "");
        return AIC_ENOMEM;
    }

    USB_AIDXPF(0x145289, "ddp", "TRB_MAP_SIZE %d, trbChainDesc idx: %d addr %p selected\n",
            TRB_MAP_SIZE, i, trbChainDesc);

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
        USB_AIDXPF(0x0c1139, "s", "No free TRBs%s\n", "");
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
    trbChainDesc->trbDMAAddr =
        (uint32_t)(uintptr_t)((DmaTrb*)pD->trbDMAPoolAddr+ trbChainDesc->start);
    trbChainDesc->isoError = 0;


    USB_AIDXPF(0x02d26a, "ddddppx", "start %d end %d nTrbs %d"
        " mSize %d chan %p trbArS %p tDmaS %08x alloc\n",
    trbChainDesc->start, trbChainDesc->end,
    trbChainDesc->numOfTrbs, trbChainDesc->mapSize,
    trbChainDesc->channel, trbChainDesc->trbPool,
    trbChainDesc->trbDMAAddr);

    /*Chain Descriptor work as FIFO, first was added first will be armed
     * and serviced*/
    listAddTail(&trbChainDesc->chainNode, &channel->trbChainDescList);

    channel->numOfTrbChain++;
    for(j= 0; j< count; j++ )
        pD->trbChainFreeMap[i-j]= 0xFF;

    *chain = trbChainDesc;
    return 0;
}

static void sgdmShowTrbChain(DmaTrbChainDesc * trbChainDesc)
{
    uint16_t i = 0;

    USB_AIDXPF(0x0c472a, "pp", "TRB Chain %p for channel %p\n",
            trbChainDesc, trbChainDesc->channel);

    USB_AIDXPF(0x1b580c, "s", "idx    | trb size | trb addr | "
            "FLAG: NORMAL LINK CHAIN  ALL %s\n", " ");

    for (i = 0; i < trbChainDesc->numOfTrbs+2; i++) {
        USB_AIDXPF(0x08e738, "dxxdddx", "%02d     | %08x | %08x |"
                "         %d      %d     %d    %08x\n",
        i, trbChainDesc->trbPool[i].dmaSize & TD_SIZE_MASK,
        trbChainDesc->trbPool[i].dmaAddr,
        (trbChainDesc->trbPool[i].ctrl & TD_TYPE_NORMAL) ? 1 : 0,
        (trbChainDesc->trbPool[i].ctrl & TD_TYPE_LINK) ? 1 : 0,
        (trbChainDesc->trbPool[i].ctrl & TDF_CHAIN_BIT) ? 1 : 0,
        trbChainDesc->trbPool[i].ctrl);
    }
}

/******************************************************************************
 * Private Driver functions for SGDMA
 *****************************************************************************/
/**
 * Function fill in TRBs in trbChainDesc
 */
static uint32_t
usb_dma_NewTdHw(DmaController * pD,
                DmaTrbChainDesc * trbChainDesc)
{
    uint32_t i = 0;
    uint32_t tmp = 0;

    uint32_t startAddress = trbChainDesc->dwStartAddress;
    DmaChannel *channel = trbChainDesc->channel;
    uint32_t dataSize = channel->maxTrbLen;

    if (trbChainDesc->numOfTrbs > 1) {
        for (i = 0; i < (trbChainDesc->numOfTrbs - 1); i++) {
            if(channel->dmultEnabled) {
                /* for isochronous transfer in host mode each TRB should
         * be build separately becaouse each packet can have
         * different size */
                if(trbChainDesc->framesDesc && pD->isHostCtrlMode && channel->isIsoc) {
                    BUILD_NORMAL_TRB_NO_IOC(trbChainDesc->trbPool[i],
                        trbChainDesc->dwStartAddress + trbChainDesc->framesDesc[i].offset,
                        trbChainDesc->framesDesc[i].length, 0);
            continue;
                } else {
                    BUILD_NORMAL_TRB_NO_IOC(trbChainDesc->trbPool[i],
                                                   startAddress, dataSize, 0);
                }
            } else {
                BUILD_NORMAL_TRB_NO_IOC_CHAIN(trbChainDesc->trbPool[i],
                                                    startAddress, dataSize, 0);
            }

            startAddress += dataSize;
            tmp += dataSize;
        }
    }

    /*Only for ISOC and Host mode*/
    if(trbChainDesc->framesDesc && pD->isHostCtrlMode && channel->isIsoc) {
       BUILD_NORMAL_TRB(trbChainDesc->trbPool[i],
           trbChainDesc->dwStartAddress + trbChainDesc->framesDesc[i].offset,
           trbChainDesc->framesDesc[i].length, 0);
    } else {
        BUILD_NORMAL_TRB(trbChainDesc->trbPool[i], startAddress,
                              trbChainDesc->len - tmp, 0);
    }

    trbChainDesc->lastTrbIsLink =0;
    if (channel->dmultEnabled) {
        if(channel->isIsoc) {
            /* In Isochronouse transfer DMA should have always prepared
         * TRB for next data. In order to ensure that firmware should
         * loop the last trb chain. If firmware does'n have time to
         * prepare next set of trbs then will be used again the last
         * trb chain.
             */
            trbChainDesc->lastTrbIsLink =1;
            BUILD_LINK_TRB(trbChainDesc->trbPool[i+1] ,
                              trbChainDesc->trbDMAAddr);
        } else {
            BUILD_EMPTY_TRB(trbChainDesc->trbPool[i+1]);
        }
    }

    sgdmShowTrbChain(trbChainDesc);
    dcache_invcc_by_range((int32_t)trbChainDesc->trbPool,
               DCLINE_GRANULARITY((int32_t)trbChainDesc->trbPool,
               sizeof(DmaTrb) * (trbChainDesc->numOfTrbs + 2)));

    return AIC_SUCCESS;
}

/**
 * Function starts DMA transfer
 */
static void usb_dma_ArmTd(DmaController * pD,
                    DmaTrbChainDesc * trbChainDesc)
{
    uint32_t epStsUpdate;
    DmaChannel * channel = trbChainDesc->channel;

    // initialize USB controller for data transfer
    writel_relaxed(&pD->regs->ep_sel,
            (uint32_t) (channel->isDirTx | channel->hwUsbEppNum));

    /* If DMA is not started than firmware can simple prepare and arm transfer*/
    if(channel->status == CUSBDMA_STATUS_FREE) {
        uint32_t cfg =0;

        USB_AIDXPF(0x18d6e3, "ppsdx", "Set DORBEL ChNr-chain-Dir-EpNr-traddr: 0x%p-0x%p-%s-%d-0x%x\n",
                    channel, trbChainDesc,
                    (channel->isDirTx) ? "Tx" : "Rx",
                    channel->hwUsbEppNum, trbChainDesc->trbDMAAddr);

        writel_relaxed(&pD->regs->ep_sts, DMARF_EP_TRBERR);
        writel_relaxed(&pD->regs->traddr, (uint32_t) trbChainDesc->trbDMAAddr);

        /* Before starting DMA firmware clear old TRBERROR
     * interrupt that could not be handled */
        writel_relaxed(&pD->regs->ep_sts, DMARF_EP_TRBERR);

        cfg = readl_relaxed(&pD->regs->ep_cfg);

        writel_relaxed(&pD->regs->ep_cmd, DMARF_EP_DRDY);
        writel_relaxed(&pD->regs->ep_sts, DMARF_EP_TRBERR);
        if(!(cfg & DMARV_EP_ENABLED)) {
            cfg |= DMARV_EP_ENABLED;
            writel_relaxed(&pD->regs->ep_cfg, (uint32_t)cfg);
        }

    } else {
        DmaTrbChainDesc * startedChain;
        /*Firmware has to replace last TRB from last transfer to link to next transfer*/

        /**
         * Check if we have at least two trbChainDesc in list. It should be always
         * meet in this place
         **/
        if(channel->trbChainDescList.prev != channel->trbChainDescList.next) {
            DmaTrb * lastPrevTrb;
            /*get one before last trbChainDesc  */
            startedChain =
                usb_dma_ListGetTrbChainDescEntry(channel->trbChainDescList.prev->prev);
            /*Last TRB in chain*/
            lastPrevTrb = &startedChain->trbPool[startedChain->numOfTrbs];

            /* If previous transfer hasn't finished in the meantime then we
         * link new TRB Chain, else we start transfer again.*/
            startedChain->lastTrbIsLink =1;
            BUILD_LINK_TRB((*lastPrevTrb) , trbChainDesc->trbDMAAddr);
            sgdmShowTrbChain(startedChain);
            dcache_invcc_by_range((int32_t)startedChain,
                            DCLINE_GRANULARITY((int32_t)startedChain,
                            sizeof(DmaTrb) * (startedChain->numOfTrbs + 2)));
            epStsUpdate = readl_relaxed(&pD->regs->ep_cmd);
            if(!(epStsUpdate & DMARF_EP_DRDY)) {
                writel_relaxed(&pD->regs->traddr, (uint32_t) trbChainDesc->trbDMAAddr);
                writel_relaxed(&pD->regs->ep_cmd, DMARF_EP_DRDY);
                writel_relaxed(&pD->regs->ep_sts, DMARF_EP_TRBERR);
            }
        }
    }
}

/**
 * Function free trbChainDesc and inform upper layer that transfer
 * has been finished
 */
static uint32_t
usb_dma_UpdateDescBuffer(DmaController * pD,
                         DmaTrbChainDesc * trbChainDesc,
                         uint16_t status)
{

    uint32_t i;
    uint32_t temp;
    DmaChannel * channel= trbChainDesc->channel;

    if(channel->isIsoc ) {
        if(trbChainDesc->lastTrbIsLink) {
            if(channel->trbChainDescList.prev == channel->trbChainDescList.next) {
                /*New trb chain descriptor isn't ready
                 * and only one TRB chain is linked,
                 * so firmware will use this one and not
                 * inform user application about finishing */
                 return AIC_SUCCESS;
            }
        }
    }

    for (i = 0; i < trbChainDesc->numOfTrbs; i++) {
        uint32_t size = ((uint32_t)trbChainDesc->trbPool[i].dmaSize) & TD_SIZE_MASK;
        // get transfer descriptor from ring
        if(trbChainDesc->framesDesc) {
            if(trbChainDesc->isoError)
                trbChainDesc->framesDesc[i].length  =0;
            else
                trbChainDesc->framesDesc[i].length = size;
        }

        trbChainDesc->actualLen +=  size;
    }

    if(trbChainDesc->isoError) {
        trbChainDesc->actualLen =0;
    }

    temp = readl_relaxed(&pD->regs->ep_sel);

    /* This value can by read in complete callback function */
    channel->lastTransferLength = trbChainDesc->actualLen;

    USB_AIDXPF(0x02b7b0, "pd", "Complete callback channel-actualLen %p-%d\n",
            channel, trbChainDesc->actualLen);

    if(!trbChainDesc->lastTrbIsLink){
        if(trbChainDesc->chainNode.next) {
            usb_dma_FreeTrbChain(pD, trbChainDesc);
            trbChainDesc = NULL;
        }
    }

    if(channel->trbChainDescList.prev == &channel->trbChainDescList)
        channel->status = CUSBDMA_STATUS_FREE;

    if (pD->callback.complete) {
        pD->callback.complete(pD->parent, channel->hwUsbEppNum,
                channel->isDirTx);
    }
    /* In case DMULT mode and when last TRB is Link TRB and
     * trbChainDesc->lastTrbIsLink is set firmware can free
     * TRB Chain after complete function in order to avoid
     * rewrite Link TRB in complete function */
    if(trbChainDesc && trbChainDesc->lastTrbIsLink) {
        if(trbChainDesc->chainNode.next)
            usb_dma_FreeTrbChain(pD, trbChainDesc);
    }
    writel_relaxed(&pD->regs->ep_sel, temp);
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

    USB_AIDXPF(0x04fdec, "", "Initialization DMA component\n");
    memset((void*) ctrl, 0, sizeof(DmaController));
    memset((void*)config->trbAddr, 0, TRB_POOL_SIZE);

    ctrl->trbDMAPoolAddr = config->trbDmaAddr;
    ctrl->trbPoolAddr = config->trbAddr;

    ctrl->regs = (DmaRegs*) config->regBase;
    ctrl->cfg = *config;
    ctrl->callback = *callbacks;
    ctrl->isHostCtrlMode = 0;
    return AIC_SUCCESS;
}

int usb_dma_start(DmaController * ctrl)
{
    uint8_t i =0;
    if (!ctrl)
        return AIC_EINVAL;

    USB_AIDXPF(0x0c0d41, "", "Starting DMA component\n");
    ctrl->dmaMode = DMA_MODE_CHANNEL_INDIVIDUAL;
    if ((ctrl->cfg.dmaModeRx & ctrl->cfg.dmaModeTx) == 0xFFFF) {
        writel_relaxed(&ctrl->regs->conf, DMARF_DMULT);
        ctrl->dmaMode = DMA_MODE_GLOBAL_DMULT;
    } else if ((ctrl->cfg.dmaModeRx | ctrl->cfg.dmaModeTx) == 0x0000) {
        writel_relaxed(&ctrl->regs->conf, DMARF_DSING);
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
    dma_ien = readl_relaxed(&regs->ep_ien);
    if (isDirTx) {
        if (ctrl->tx[hwEpNum].status >= CUSBDMA_STATUS_FREE) {
            USB_AIDXPF(0x0134f0, "pd", "Channel 0x%p dedicated for Ep %d is already reserved\n",
                       &ctrl->tx[hwEpNum], hwEpNum);
            return NULL;
        }
        channel = &ctrl->tx[hwEpNum];
        channel->isDirTx = 0x80;
        dma_ien |= (0x01 << (hwEpNum + 16));
        dmaMode = ctrl->cfg.dmaModeTx;
    } else {
        if (ctrl->rx[hwEpNum].status >= CUSBDMA_STATUS_FREE) {
            USB_AIDXPF(0x0134f0, "pd", "Channel 0x%p dedicated for Ep %d is already reserved\n",
                       &ctrl->rx[hwEpNum], hwEpNum);
            return NULL;
        }
        channel = &ctrl->rx[hwEpNum];
        channel->isDirTx = 0x00;
        dma_ien |= (0x01 << hwEpNum);
        dmaMode = ctrl->cfg.dmaModeRx;
    }

    channel->isIsoc = 0;
    if(isIsoc)
           channel->isIsoc = 1;

    USB_AIDXPF(0x10cace, "pd", "Channel 0x%p has been allocated for ep %d\n", channel, hwEpNum);
    listInit(&channel->trbChainDescList);
    channel->numOfTrbChain=0;
    channel->controller = ctrl;
    channel->dmultGuard = 0;
    channel->status = CUSBDMA_STATUS_FREE;
    channel->hwUsbEppNum = hwEpNum;

    writel_relaxed(&regs->ep_sel, (uint32_t) hwEpNum | channel->isDirTx);
    cfg = readl_relaxed(&regs->ep_cfg);

    if (ctrl->dmaMode == DMA_MODE_CHANNEL_INDIVIDUAL) {
        if (dmaMode & (1 << hwEpNum)) {
            cfg |= DMARV_EP_DMULT;
        } else {
            cfg |= DMARV_EP_DSING;
        }
    }

    writel_relaxed(&regs->ep_sts, DMARF_EP_IOC | DMARF_EP_ISP |
            DMARF_EP_TRBERR /*| DMARF_EP_ISOERR*/);
    writel_relaxed(&regs->ep_cfg, (uint32_t) DMARV_EP_ENABLED | cfg);
    writel_relaxed(&regs->ep_ien, dma_ien);
    writel_relaxed(&regs->ep_sts_en, DMARF_EP_TRBERR
            /*| DMARF_EP_ISOERR| DMARF_EP_DESCMIS | DMARF_EP_OUTSMM*/);

    return channel;
}

uint32_t usb_dma_channel_abort(DmaController * ctrl, void * channel)
{
    DmaChannel * dmaChannel = (DmaChannel*) channel;
    uint32_t cfg=0;
    AIC_INT_SAVE_AREA

    if (!ctrl || !channel)
        return AIC_EINVAL;

    USB_AIDXPF(0x123a2c, "p", "Channel 0x%p has been aborted\n", channel);

    AIC_INT_DISABLE
    /*Disable DMA*/
    writel_relaxed(&ctrl->regs->ep_sel,
            (uint32_t) (dmaChannel->isDirTx | dmaChannel->hwUsbEppNum));
    cfg = readl_relaxed(&ctrl->regs->ep_cfg);
    cfg &= ~DMARV_EP_ENABLED;
    writel_relaxed(&ctrl->regs->ep_cfg, (uint32_t)cfg);
    /*Clear all interrupts for this channel*/
    writel_relaxed(&ctrl->regs->ep_sts, 0xFFFFFFFF);
    AIC_INT_RESTORE

    while(dmaChannel->trbChainDescList.next != &dmaChannel->trbChainDescList) {
        usb_dma_FreeTrbChain(ctrl,
        usb_dma_ListGetTrbChainDescEntry(dmaChannel->trbChainDescList.next));
    }

    dmaChannel->status = CUSBDMA_STATUS_FREE;
    return AIC_SUCCESS;
}

uint32_t usb_dma_channel_release(DmaController *ctrl, void *channel)
{
    DmaChannel * dmaChannel = (DmaChannel *) channel;
    uint32_t dmaIen;

    AIC_INT_SAVE_AREA

    USB_AIDXPF(0x06c5bb, "p", "Dma channel release ch: %p\n", channel);
    if (!channel || !ctrl)
        return AIC_EINVAL;

    /* Disable endpoint and interrupt for DMA. Probably it should be
     * in dmaChannelAbort */
    dmaIen = readl_relaxed(&ctrl->regs->ep_ien);
    if (dmaChannel->isDirTx) {
        dmaIen &= ~(0x01 << (dmaChannel->hwUsbEppNum + 16));
    } else {
        dmaIen &= ~(0x01 << dmaChannel->hwUsbEppNum);
    }

    AIC_INT_DISABLE
    writel_relaxed(&ctrl->regs->ep_sel,
            (uint32_t) dmaChannel->hwUsbEppNum | dmaChannel->isDirTx);
    writel_relaxed(&ctrl->regs->ep_cfg, (uint32_t) 0);

    writel_relaxed(&ctrl->regs->ep_ien, dmaIen);
    writel_relaxed(&ctrl->regs->ep_sts_en, 0x0);
    writel_relaxed(&ctrl->regs->ep_cmd, DMARF_EP_EPRST);

    writel_relaxed(&ctrl->regs->ep_sts, DMARF_EP_IOC | DMARF_EP_ISP |
            DMARF_EP_TRBERR | DMARF_EP_ISOERR);

    if(dmaChannel->status >= CUSBDMA_STATUS_BUSY)
        dmaChannel->status = CUSBDMA_STATUS_ABORT;
    AIC_INT_RESTORE

    usb_dma_channel_abort(ctrl, channel);

    while(dmaChannel->trbChainDescList.next != &dmaChannel->trbChainDescList)
        usb_dma_FreeTrbChain(ctrl,
        usb_dma_ListGetTrbChainDescEntry(dmaChannel->trbChainDescList.next));

    USB_AIDXPF(0x0a21c6, "p", "Channel 0x%p has been released\n", channel);

    dmaChannel->status = CUSBDMA_STATUS_UNKNOW;
    return AIC_SUCCESS;
}

uint32_t
usb_dma_channel_program(DmaController * ctrl, void * channel,
            uint16_t packetSize, uintptr_t dmaAddr,
            uint32_t len, void * framesDesc,
            uint32_t framesNumber)
{
    DmaTrbChainDesc *trbChainDesc;
    DmaChannel * dmaChannel;
    uint32_t numOfTrbs;
    uint8_t retval;

    AIC_INT_SAVE_AREA

    if (!channel) {
        USB_AIDXPF(0x00a49c, "", "usb_dma_channel_program  channel is NULL\n");
        return AIC_EINVAL;
    }

    dmaChannel = (DmaChannel*) channel;

    USB_AIDXPF(0x0f24d5, "pdsdlxd", "DMA prepare for channel: 0x%p ep%d-%s packetSize %d, dmaAddr 0x%lx length %d\n",
    dmaChannel, dmaChannel->hwUsbEppNum, dmaChannel->isDirTx ? "Tx" : "Rx",
    packetSize, dmaAddr, len);

    if(framesDesc && ctrl->isHostCtrlMode && dmaChannel->isIsoc)
        numOfTrbs = framesNumber;
    else
        numOfTrbs = divRoundUp(len, dmaChannel->maxTrbLen);

    AIC_INT_DISABLE
    retval = usb_dma_AllocTrbChain(ctrl, channel, numOfTrbs, &trbChainDesc);
    if(retval) {
        AIC_INT_RESTORE
        USB_AIDXPF(0x1da178, "x", "usb_dma_AllocTrbChain fail ret = 0x%x\n", retval);
        return retval;
    }

    trbChainDesc->dwStartAddress = dmaAddr;
    trbChainDesc->len = len;
    trbChainDesc->framesDesc = framesDesc;
    dmaChannel->wMaxPacketSize = packetSize;

    usb_dma_NewTdHw(ctrl, trbChainDesc);
    dmaChannel->lastTransferLength = 0;
    usb_dma_ArmTd(ctrl, trbChainDesc);
    dmaChannel->status = CUSBDMA_STATUS_BUSY;
    AIC_INT_RESTORE

    return AIC_SUCCESS;
}

/*
 * this function should be called when system indicates interrupt for DMA.
 */
void usb_dma_isr(DmaController *ctrl)
{
    DmaTrbChainDesc *trbChainDesc = NULL;
    DmaChannel *channel = NULL;
    uint32_t epSts, epIsts;
    DmaRegs * regs;
    int i;

    if (!ctrl)
        return;

    regs = ctrl->regs;

    // Check interrupts for all endpoints
    epIsts = readl_relaxed(&regs->ep_ists);
    if (epIsts == 0)
        return;

    USB_AIDXPF(0x080930, "x", "DMAIRQ ep_ists SFR: %x\n", epIsts);
    // check all endpoints interrups
    for (i = 0; i < 32; i++) {
        uint8_t isDirTx;
        uint8_t epNum;

        if (!(epIsts & (1 << i)))
            continue;

        isDirTx = i > 15 ? DMARD_EP_TX : 0u;
        epNum = isDirTx ? i - 16 : i;

        if (epNum >= MAX_DMA_CHANNELS)
            continue;
        writel_relaxed(&regs->ep_sel, epNum | isDirTx);
        epSts = readl_relaxed(&regs->ep_sts);

        if (isDirTx) /*transmit data*/{
            channel = &ctrl->tx[epNum];
        } else {
            channel = &ctrl->rx[epNum];
        }

        USB_AIDXPF(0x119bc4, "xds", "DMA IRQ epSts %08x epNum: %d epDir: %s\n",
                epSts, epNum, (isDirTx ? "TX" : "RX"));

        if (epSts & DMARF_EP_TRBERR) {
            USB_AIDXPF(0x16159b, "s", "DMAIRQ: TRBERR %s\n", "");
            writel_relaxed(&regs->ep_sts, DMARF_EP_TRBERR);
        }

        if ((epSts & DMARF_EP_IOC) || (epSts & DMARF_EP_ISP)
            || (channel->dmultGuard & DMARF_EP_IOC)
            || (channel->dmultGuard & DMARF_EP_ISP)) {
retransmit:
            trbChainDesc =
                usb_dma_ListGetTrbChainDescEntry(channel->trbChainDescList.next);
            writel_relaxed(&regs->ep_sts, DMARF_EP_IOC | DMARF_EP_ISP);

            /*Only For DMULT Transfer*/
            if (!(epSts & DMARF_EP_TRBERR) && channel->dmultEnabled &&
                !trbChainDesc->lastTrbIsLink) {
                USB_AIDXPF(0x0ce9ca, "s", "DMAIRQ: IOC & ISP wait for TRBERROR %s\n", " ");

                /*For EP OUT in Device mode firmware can't wait for TRB ERROR*/
                if(!ctrl->isHostCtrlMode && !channel->isDirTx) {
                    uint32_t cfg;
                    channel->dmultGuard = 0;
                    cfg = readl_relaxed(&regs->ep_cfg);
                    cfg &= ~DMARV_EP_ENABLED;
                    writel_relaxed(&regs->ep_cfg, (uint32_t)cfg);
                    writel_relaxed(&regs->ep_sts, DMARF_EP_TRBERR);
                    usb_dma_UpdateDescBuffer(ctrl, trbChainDesc, 0);
                    break;
                }

                channel->dmultGuard = epSts;
                break;
            } else
                channel->dmultGuard = 0;

            usb_dma_UpdateDescBuffer(ctrl, trbChainDesc, 0);
        }

        // stop program if critical error: Data overflow
        //assert(!(ep_sts & USBRF_EP_OUTSMM));
        if (epSts & DMARF_EP_OUTSMM) {
            writel_relaxed(&regs->ep_sts, DMARF_EP_OUTSMM);
            USB_AIDXPF(0x0048d4, "s", "DMAIRQ: OUTSMM%s\n", "");
        }

        if (epSts & DMARF_EP_DESCMIS) {
            writel_relaxed(&regs->ep_sts, DMARF_EP_DESCMIS);
            USB_AIDXPF(0x034ba7, "s", "DMAIRQ: DESCMIS%s\n", "");
        }

        if (epSts & DMARF_EP_ISOERR) {
            writel_relaxed(&regs->ep_sts, DMARF_EP_ISOERR);
            trbChainDesc =
                usb_dma_ListGetTrbChainDescEntry(channel->trbChainDescList.next);
            trbChainDesc->isoError =1;
            USB_AIDXPF(0x094d32, "s", "DMAIRQ: ISOERR%s\n", "");
            goto retransmit;
        }
    }
}

CUSBDMA_Status
usb_dma_get_channel_status(DmaController *ctrl, void *channel)
{
    DmaChannel * ch = (DmaChannel*)channel;

    if (!ctrl || !channel)
        return CUSBDMA_STATUS_UNKNOW;

    /* If DMA has been armed and has not finished yet then
     * update status to *_ARMED. This procedure allows to detect
     * that DMA HW is in progress.
     */
    if(ch->status >= CUSBDMA_STATUS_BUSY) {
        uint32_t dorbel, status;
        writel_relaxed(&ctrl->regs->ep_sel,
            (uint32_t) (ch->isDirTx | ch->hwUsbEppNum));

        dorbel = readl_relaxed(&ctrl->regs->ep_cmd);
        status = readl_relaxed(&ctrl->regs->ep_sts);

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
        return 0;

    return ch->lastTransferLength;
}

void usb_dma_set_max_length(void *pD, void *channel, uint32_t size)
{
    DmaChannel * ch = (DmaChannel*) channel;

    if(ch->dmultEnabled) {
        ch->maxTrbLen = size;
    } else {
        ch->maxTrbLen  = (size > TD_SING_MAX_TRB_DATA_SIZE) ?
            TD_SING_MAX_TRB_DATA_SIZE : size;
    }
}
