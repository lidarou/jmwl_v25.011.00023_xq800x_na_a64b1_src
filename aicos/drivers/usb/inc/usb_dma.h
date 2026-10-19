/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef _USB_DMA_INT_H
#define _USB_DMA_INT_H

#include "usb_dma_regs.h"

/** This value should be module 8*/
#define NUM_OF_TRB 512

/**
 * Number of TRBs that will be allocated for DMA component. This trb will
 * be dynamically managed by DMA driver
 */
#define TRB_POOL_SIZE (sizeof(DmaTrb) * (NUM_OF_TRB))

/**
 * Number of channel supported by DMA driver
 */
 #define MAX_DMA_CHANNELS 9

/*
 * Number of byte need for holding information about TRBs allocated from TRB pool
 **/
#define TRB_MAP_SIZE ((NUM_OF_TRB+(sizeof(uint8_t)*8)-1)/(sizeof(uint8_t)*8))

        /**
 * Number of element for holding information about TRB chain
 * We assume that for every channel for IN and OUT direction we can
 * allocate average one TRBS chain.
 */
#define TAB_SIZE_OF_DMA_CHAIN MAX_DMA_CHANNELS * 2

/** Configuration of DMA.
 *  Object of this type is used for probe and init functions when checking
 *  memory requirements of device object and for hardware controller configuration.
 *  Size of device object in bytes is returned on probe function call in
 *  CUSBDMA_SysReq.privDataSize field.
 */
typedef struct {
    uintptr_t regBase;
    uint16_t dmaModeTx;
    uint16_t dmaModeRx;
    void * trbAddr;
    /** Address of to physical memory for TRBs buffer */
    uintptr_t trbDmaAddr;
} CUSBDMA_Config;

typedef void (*CUSBDMA_CbDmaComplete)(void *pD, uint8_t epNum, uint8_t dir);
typedef struct {
    CUSBDMA_CbDmaComplete complete;
} CUSBDMA_Callbacks;

/*Describe One trb*/
typedef struct {
    uint32_t dmaAddr;
    uint32_t dmaSize;
    uint32_t ctrl;
} DmaTrb;

struct DmaController;
struct DmaChannel;
struct DmaTrbChainDesc;

typedef struct CUSBDMA_ListHead
{
    /** pointer to next element in list */
    struct CUSBDMA_ListHead* next;
    /** pointer to previous element in list */
    struct CUSBDMA_ListHead* prev;
}  CUSBDMA_ListHead;

#ifndef ListHead
#define ListHead CUSBDMA_ListHead
#endif

/**DMA Channel Status available values*/
typedef enum {
    /**channel unallocated*/
    CUSBDMA_STATUS_UNKNOW,
    /**channel allocated but not busy, no errors*/
    CUSBDMA_STATUS_FREE,
    /**DMA error or USB error - transfer aborted*/
    CUSBDMA_STATUS_ABORT,
    /**channel busy - transfer in progress*/
    CUSBDMA_STATUS_BUSY,
    /**DMA has been armed and Dorbell or DMA BUSY bit is still set.
       If those bits are set it indicate that DMA is armed and data
       has not been transfered yet
    */
    CUSBDMA_STATUS_ARMED
} CUSBDMA_Status;

typedef struct DmaChannel {
    struct DmaController * controller;
    uint16_t wMaxPacketSize;
    uint8_t hwUsbEppNum; /*USB EP number. It can be different then channel number */
    uint8_t isDirTx;
    /*Maximum size for Transfer Descriptor*/
    uint32_t maxTdLen;
    /*Maximum size for single Transfer Request Block */
    uint32_t maxTrbLen;
    CUSBDMA_Status status;

    /*TRB management*/
    void * priv; /*for private use*/
    /*For DMULT mode firmware must wait for TRBERR interrupt. After TRBERR firmware can finish transfer (ADMA stop)
     * this filed hold rasising interrupt before TRBERR (ISP and IOC) */
    uint32_t dmultGuard;
    uint8_t dmultEnabled; /*field holds dmult configuration for channel*/
    uint8_t numOfTrbChain;
    ListHead trbChainDescList;
    /** Field hold information about number of bytes received or sent in last
     * transfer*/
    uint32_t lastTransferLength;
    uint8_t isIsoc;
} DmaChannel;

/*Describe packet for one isochronous interval*/
typedef struct DmaTrbFrameDesc {
     /**
     * For IN (RX) isochronous transfer this field holds information about received length of packet,
     * and for OUT (TX) direction this field inicate length of submiting frame
    */
    uint32_t length;
    /**
     * For IN (RX) direction this field indicates inital address in buffer where received packet should
     * be placed and for OUT (TX) direction this field indicates starting address in buffer for sending
     * packet
    */
    uint32_t offset;
}  DmaTrbFrameDesc;


typedef struct DmaTrbChainDesc {
    uint8_t reserved;
    DmaChannel * channel;
    DmaTrb * trbPool;
    uint32_t trbDMAAddr;
    uint32_t len;
    uint32_t dwStartAddress;
    uint32_t actualLen;
    uint8_t isoError;
    /** Finish transfer and inform USB driver when IOC event has been detected.
	This filed is used for DMULT mode. By default in DMULT mode DMA finish
	transfer on TRBERR event, but when firmware link many TRB Chain Descriptors
	for one channel then last bad TRB descriptor from Chain Descriptor will be
	removed, and transfer should be finished on IOC event */
    uint8_t lastTrbIsLink;
    uint32_t mapSize; /*Number of elements divided by 8 reserved in TRBs map*/
    uint32_t numOfTrbs; /*Number of TRB in chain without LINK and bad TRB*/
    uint32_t start; /* index of oldest element              */
    uint32_t end; /* index at which to write new element  */
    DmaTrbFrameDesc *framesDesc; /*used only for ISOC transfer in Host mode*/
    ListHead chainNode;
} DmaTrbChainDesc;;

typedef enum {
    DMA_MODE_GLOBAL_DMULT,
    DMA_MODE_GLOBAL_DSINGL,
    DMA_MODE_CHANNEL_INDIVIDUAL,
} DmaMode;

typedef struct DmaController {
    DmaRegs * regs;
    CUSBDMA_Config cfg;
    CUSBDMA_Callbacks callback;
    DmaChannel rx[MAX_DMA_CHANNELS];
    DmaChannel tx[MAX_DMA_CHANNELS];
    DmaMode dmaMode;
    uint8_t isHostCtrlMode;
    void * parent;  //parent private data
    void * trbPoolAddr;
    uintptr_t trbDMAPoolAddr;
    uint8_t trbChainFreeMap[TRB_MAP_SIZE];
    DmaTrbChainDesc trbChainDesc[TAB_SIZE_OF_DMA_CHAIN];
} DmaController;


/*Function initialize list*/
static inline void listInit(ListHead *list) {
    list->next = list;
    list->prev = list;
}

/*Add new element to the list between prev and next*/
static inline void listAddElement(ListHead * new, ListHead * prev, ListHead * next) {
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

/*Add new entry after a specified head*/
static inline void listAdd(ListHead *new,  ListHead * head) {
    listAddElement(new, head, head->next);
}

static inline void listAddTail(ListHead *new, ListHead *head) {
    listAddElement(new, head->prev, head);
}

static inline void listDelete(ListHead * list) {
    list->next->prev = list->prev;
    list->prev->next = list->next;

    list->prev = NULL;
    list->next =NULL;
}

CUSBDMA_Status usb_dma_get_channel_status(DmaController *ctrl, void *channel);
uint32_t usb_dma_channel_program(DmaController * ctrl, void * channel,
            uint16_t packetSize, uintptr_t dmaAddr, uint32_t len,
            void * framesDesc, uint32_t framesNumber);
uint32_t usb_dma_channel_abort(DmaController * ctrl, void * channel);
void * usb_dma_channel_alloc(DmaController *ctrl, uint8_t isDirTx,
                                uint8_t hwEpNum, uint8_t isIsoc);
uint32_t usb_dma_channel_release(DmaController *ctrl, void *channel);
void usb_dma_isr(DmaController *ctrl);
void usb_dma_set_max_length(void *pD, void *channel, uint32_t size);
int usb_dma_start(DmaController * ctrl);
uint32_t usb_dma_init(DmaController * ctrl, const CUSBDMA_Config* config,
                    CUSBDMA_Callbacks * callbacks);
uint32_t usb_dma_get_actual_length(void *pD, void *channel);

#endif
