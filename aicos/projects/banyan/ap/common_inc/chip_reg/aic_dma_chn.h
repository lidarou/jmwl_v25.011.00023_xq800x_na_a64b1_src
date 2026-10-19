/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file includes chip ap dma hw request channels info.
 */


#ifndef __AIC_DMA_CHN_H__
#define __AIC_DMA_CHN_H__

#define DMA_CHN_MAX         14

typedef enum  {
	DMA_REQ_SOFTWARE = 0,
    DMA_REQ_MCU_SSP0_TX,
    DMA_REQ_MCU_SSP0_RX,
    DMA_REQ_MCU_UART0_TX,
    DMA_REQ_MCU_UART0_RX,
    DMA_REQ_MCU_UART1_TX,
    DMA_REQ_MCU_UART1_RX,
    DMA_REQ_TOP_UART0_TX,
    DMA_REQ_TOP_UART0_RX,
    DMA_REQ_TOP_LPUART_TX,
    DMA_REQ_TOP_LPUART_RX,
    DMA_REQ_TOP_IIS0_TX,
    DMA_REQ_TOP_IIS0_RX,
    DMA_REQ_MEM_QSPI_TX,
    DMA_REQ_MEM_QSPI_RX,
    DMA_REQ_HWID_MAX
} DMA_HWID;

#endif /* __AIC_DMA_CHN_H__ */
