/*****************************************************************************/
/*                                                                           */
/*         Copyright 2021 by AIC Corporation. All rights reserved.           */
/*                                                                           */
/*****************************************************************************/

#ifndef __SSPREG_H__
#define __SSPREG_H__

#include <aic_type.h>
#include <reg_base.h>

struct ssp_reg {
    __IO u32 start;
    __IO u32 cr0;
    __IO u32 cr1;
    __IO u32 cr2;
    __IO u32 int_sts;
    __IO u32 int_msk;
    __IO u32 int_clr;
    __IO u32 int_raw;
    __IO u32 sts0;
    __IO u32 soft_rst;
    __IO u32 sts1;
    __IO u8  reserved[20];
    __IO u32 fifo;
};

#define START_ENABLE            (0x1 << 0)

#define CR0_MW_CTRL_LEN         (0x1 << 31)
#define CR0_DATA_BEAT_SHIFT     (29)
#define CR0_DATA_BEAT_MASK      (0x3 << CR0_DATA_BEAT_SHIFT)
#define CR0_DATA_LEN_SHIFT      (24)
#define CR0_DATA_LEN_MASK       (0x1F << CR0_DATA_LEN_SHIFT)
#define CR0_BAUD_RATE_SHIFT     (16)
#define CR0_BAUD_RATE_MASK      (0xFF << CR0_BAUD_RATE_SHIFT)
#define CR0_HALF_DUPLEX_EN      (0x1 << 15)
#define CR0_SFRM_DEFAULT_VAL    (0x1 << 14)
#define CR0_I2S_DSP_MODE        (0x1 << 13)
#define CR0_SFRM_WIDTH_SHIFT    (8)
#define CR0_SFRM_WIDTH_MASK     (0x1F << CR0_SFRM_WIDTH_SHIFT)
#define CR0_I2S_RIGHT_ALIGN     (0x1 << 7)
#define CR0_I2S_MODE            (0x1 << 6)
#define CR0_MW_DUMMY_CYCLE      (0x1 << 5)
#define CR0_MW_MODE             (0x1 << 4)
#define CR0_SSP_MODE            (0x1 << 3)
#define CR0_CPHA                (0x1 << 2)
#define CR0_CPOL                (0x1 << 1)
#define CR0_MASTER_MODE         (0x1 << 0)

#define CR1_CS_SEL_MASK         (0x1 << 31)
#define CR1_CS_SEL_SHIFT        (31)
#define CR1_CS_TOG_MODE         (0x1 << 30)
#define CR1_START_DLY_SHIFT     (24)
#define CR1_START_DLY_MASK      (0xF << CR1_START_DLY_SHIFT)
#define CR1_DLY_NSS_SHIFT       (16)
#define CR1_DLY_NSS_MASK        (0xFF << CR1_DLY_NSS_SHIFT)
#define CR1_DLY_END_SHIFT       (8)
#define CR1_DLY_END_MASK        (0xFF << CR1_DLY_END_SHIFT)
#define CR1_DLY_INIT_SHIFT      (0)
#define CR1_DLY_INIT_MASK       (0xFF << CR1_DLY_INIT_SHIFT)

#define CR2_RXFIFO_THRD_SHIFT   (24)
#define CR2_RXFIFO_THRD_MASK    (0xF << CR2_RXFIFO_THRD_SHIFT)
#define CR2_TXFIFO_THRD_SHIFT   (16)
#define CR2_TXFIFO_THRD_MASK    (0xF << CR2_TXFIFO_THRD_SHIFT)
#define CR2_RX_CAP_EDGE         (0x1 << 9)
#define CR2_DMA_CLOSE           (0x1 << 8)
#define CR2_CAP_DATA_DLY_SHIFT  (0)
#define CR2_CAP_DATA_DLY_MASK   (0x3 << CR2_CAP_DATA_DLY_SHIFT)

#define INT_RXFIFO_FULL         (0x1 << 5)
#define INT_RXFIFO_OVER_THRD    (0x1 << 4)
#define INT_RXFIFO_EMPTY        (0x1 << 3)
#define INT_TXFIFO_BELOW_THRD   (0x1 << 2)
#define INT_TXFIFO_FULL         (0x1 << 1)
#define INT_SSP_FINISH          (0x1 << 0)
#define INT_ALL                 (INT_RXFIFO_FULL | INT_RXFIFO_OVER_THRD | \
                                    INT_RXFIFO_EMPTY | INT_TXFIFO_BELOW_THRD | \
                                    INT_TXFIFO_FULL | INT_SSP_FINISH)

#define STS_CORE_STATUS_SHIFT   (0)
#define STS_CORE_STATUS_MASK    (0xF << STS_CORE_STATUS_SHIFT)
#define STS_BUS_STATUS_SHIFT    (4)
#define STS_BUS_STATUS_MASK     (0xF << STS_BUS_STATUS_SHIFT)
#define STS_TX_FIFO_CNT_SHIFT   (8)
#define STS_TX_FIFO_CNT_MASK    (0x1F << STS_TX_FIFO_CNT_SHIFT)
#define STS_TX_FIFO_EMPTY       (0x1 << 14)
#define STS_TX_FIFO_FULL        (0x1 << 15)
#define STS_RX_FIFO_CNT_SHIFT   (16)
#define STS_RX_FIFO_CNT_MASK    (0x1F << STS_RX_FIFO_CNT_SHIFT)
#define STS_RX_FIFO_EMPTY       (0x1 << 22)
#define STS_RX_FIFO_FULL        (0x1 << 23)

#define SLV_RESET               (0x1 << 0)

#define STS1_TX_SEL_SHIFT       (0)
#define STS1_TX_SEL_MASK        (0x1F << STS1_TX_SEL_SHIFT)
#define STS1_TX_SEL_START       (0x1 << 5)
#define STS1_IN_PROGRESS        (0x1 << 6)
#define STS1_SFRM_IN_DLY        (0x1 << 7)
#define STS1_SLV_RX_CNT_SHIFT   (8)
#define STS1_SLV_RX_CNT_MASK    (0x1F << STS1_SLV_RX_CNT_SHIFT)
#define STS1_SLV_RX_FULL        (0x1 << 13)

#define SSP_FIFO_SIZE           (16)

#endif /* __SSPREG_H__ */
