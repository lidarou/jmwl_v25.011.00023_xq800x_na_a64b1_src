/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the dma phy elements used by gpio driver code.
 */

#ifndef __DMA_REG_H__
#define __DMA_REG_H__

#include <aic_bitmap.h>
#include <aic_reg_base.h>

/* common registers */
#define DMAC_IPV                            0x000
#define DMAC_AUTOCLK_EN                     0x004
#define DMAC_SUSPEND                        0x008
#define DMAC_INTEN                          0x00C
#define DMAC_CHANINT_MASKSTS                0x010
#define DMAC_CHANINT_RAWSTS                 0x014
#define DMAC_MSTCFG                         0x018
#define DMAC_BLK_DELAY                      0x01C
#define DMAC_CHANSTS0                       0x020
#define DMAC_CHANSTS1                       0x024
#define DMAC_MSTSTS                         0x028
#define DMAC_DMASTS                         0x02c
#define DMAC_COMREG_PROT                    0x0400
#define DMAC_REQ_CHANID0_PROT               0x0404
#define DMAC_REQ_CHANID1_PROT               0x0408
#define DMAC_CHANREG_PROT                   0x040C
#define DMAC_REQ_CFG(x)                     (0x1000 + 4 * (x))

/* channel registers */
#define DMAC_CHAN_REG_OFFSET                0x2000
#define DMAC_CHAN_REG_SIZE                  0x40
#define DMAC_CHAN_INTEN(x)                  (0x2000 + 0x40 * (x) + 0x000)
#define DMAC_CHAN_RAWINTSTS(x)              (0x2000 + 0x40 * (x) + 0x004)
#define DMAC_CHAN_MASKINTSTS(x)             (0x2000 + 0x40 * (x) + 0x008)
#define DMAC_CHAN_INTCLR(x)                 (0x2000 + 0x40 * (x) + 0x00C)
#define DMAC_CHAN_REQ(x)                    (0x2000 + 0x40 * (x) + 0x010)
#define DMAC_CHAN_COMCFG(x)                 (0x2000 + 0x40 * (x) + 0x014)
#define DMAC_CHAN_SUSPEND(x)                (0x2000 + 0x40 * (x) + 0x018)
#define DMAC_CHAN_MSTCFG(x)                 (0x2000 + 0x40 * (x) + 0x01C)

/* Channel Description Pool Registers */
#define DMAC_CHAN_DESC_REG_OFFSET           0x3000
#define DMAC_CHAN_DESC_REG_SIZE             0x30
#define DMAC_SAR_0(x)                       (0x3000 + 0x30 * (x) + 0x000)
#define DMAC_SAR_1(x)                       (0x3000 + 0x30 * (x) + 0x004)
#define DMAC_DAR_0(x)                       (0x3000 + 0x30 * (x) + 0x008)
#define DMAC_DAR_1(x)                       (0x3000 + 0x30 * (x) + 0x00c)
#define DMAC_LCR_0(x)                       (0x3000 + 0x30 * (x) + 0x010)
#define DMAC_LCR_1(x)                       (0x3000 + 0x30 * (x) + 0x014)
#define DMAC_SSR_0(x)                       (0x3000 + 0x30 * (x) + 0x018)
#define DMAC_SSR_1(x)                       (0x3000 + 0x30 * (x) + 0x01c)
#define DMAC_DSR_0(x)                       (0x3000 + 0x30 * (x) + 0x020)
#define DMAC_DSR_1(x)                       (0x3000 + 0x30 * (x) + 0x024)
#define DMAC_LLR_0(x)                       (0x3000 + 0x30 * (x) + 0x028)
#define DMAC_LLR_1(x)                       (0x3000 + 0x30 * (x) + 0x02c)


/*** dma common register bits define ***/
/* DMAC_AUTOCLK_EN */
#define BIT_DMAC_CNT_AUTOCLK_EN             BIT(5)
#define BIT_DMAC_AXI_AUTOCLK_EN             BIT(4)
#define BIT_DMAC_REG_W_AUTOCLK_EN           BIT(3)
#define BIT_DMAC_REQ_CID_AUTOCLK_EN         BIT(2)
#define BIT_DMAC_REG_CFG_AUTOCLK_EN         BIT(1)
#define BIT_DMAC_GLB_CFG_AUTOCLK_EN         BIT(0)

/*REG_DMAC_SUSPEND*/
#define BIT_DMAC_SUSPEND_STS                BIT(16)
#define BIT_DMAC_SUSPEND_REQ                BIT(0)

/*REG_DMAC_INTEN*/
#define BIT_DMAC_INTEN                      BIT(0)

/*REG_DMAC_MSTCFG*/
#define BIT_DMAC_MST_SRC_ENDIAN             BIT(17)
#define BIT_DMAC_MST_DES_ENDIAN             BIT(16)
#define BIT_DMAC_MST_SRC_OUTSD(x)           (((x) & 0xf) << 4)
#define BIT_DMAC_MST_DES_OUTSD(x)           (((x) & 0xf))

/*REG_DMAC_BLK_DELAY*/
#define BIT_DMAC_BLK_DELAY(x)               (((x) & 0xffff))

/*REG_DMAC_CHANSTS0*/
#define BIT_DMAC_CHANSTS0(x)                (((x) & 0xffffffff))

/*REG_DMAC_CHANSTS1*/
#define BIT_DMAC_CHANSTS1(x)                (((x) & 0xffffffff))

/*REG_DMAC_MSTSTS*/
#define BIT_DMAC_MSTSTS(x)                  (((x) & 0xffffffff))

/*REG_DMAC_DMASTS*/
#define BIT_DMAC_DMASTS(x)                  (((x) & 0xffffffff))

/*REG_DMAC_COMREG_PROT*/
#define BIT_DMAC_COMREG_PROT(x)             (((x) & 0xffffffff))

/*REG_DMAC_REQ_CHANID0_PROT*/
#define BIT_DMAC_REQ_CHANID0_PROT(x)        (((x) & 0xffffffff))

/*REG_DMAC_REQ_CHANID1_PROT*/
#define BIT_DMAC_REQ_CHANID1_PROT(x)        (((x) & 0xffffffff))

/*REG_DMAC_CHANREG_PROT*/
#define BIT_DMAC_CHANREG_PROT(x)            (((x) & 0xffffffff))

/*REG_DMAC_REQX_CFG*/
#define BIT_REQX_CHANID(x)                  (((x) & 0x3f))


/*** dma channel register bits define ***/
/* DMAC_CHAN_INTEN */
#define BIT_DMAC_CHAN_ERROR_INTEN           BIT(4)
#define BIT_DMAC_CHAN_LLT_INTEN             BIT(3)
#define BIT_DMAC_CHAN_FRAME_INTEN           BIT(2)
#define BIT_DMAC_CHAN_MBLK_INTEN            BIT(1)
#define BIT_DMAC_CHAN_BLK_INTEN             BIT(0)

/* DMAC_CHAN_RAWINTSTS */
#define BIT_DMAC_CHAN_ERROR_TYPE(x)         (((x) & 0xf) << 16)
#define BIT_DMAC_CHAN_ERROR_RAWINT          BIT(4)
#define BIT_DMAC_CHAN_LLT_DONE_RAWINT       BIT(3)
#define BIT_DMAC_CHAN_FRAME_DONE_RAWINT     BIT(2)
#define BIT_DMAC_CHAN_MBLK_DONE_RAWINT      BIT(1)
#define BIT_DMAC_CHAN_BLK_DONE_RAWINT       BIT(0)

/* DMAC_CHAN_MASKINTSTS */
#define BIT_DMAC_CHAN_ERROR_MASKINT         BIT(4)
#define BIT_DMAC_CHAN_LLT_DONE_MASKINT      BIT(3)
#define BIT_DMAC_CHAN_FRAME_DONE_MASKINT    BIT(2)
#define BIT_DMAC_CHAN_MBLK_DONE_MASKINT     BIT(1)
#define BIT_DMAC_CHAN_BLK_DONE_MASKINT      BIT(0)

/* DMAC_CHAN_INTCLR */
#define BIT_DMAC_CHAN_ERROR_INTCLR          BIT(4)
#define BIT_DMAC_CHAN_LLT_DONE_INTCLR       BIT(3)
#define BIT_DMAC_CHAN_FRAME_DONE_INTCLR     BIT(2)
#define BIT_DMAC_CHAN_MBLK_DONE_INTCLR      BIT(1)
#define BIT_DMAC_CHAN_BLK_DONE_INTCLR       BIT(0)

/* DMAC_CHAN_REQ */
#define BIT_DMAC_CHAN_SW_REQ                BIT(1)
#define BIT_DMAC_CHAN_REQSTS                BIT(0)

/* DMAC_CHAN_COMCFG */
#define BIT_DMAC_CHAN_LLT_READY             BIT(17)
#define BIT_DMAC_CHAN_LLT_EN                BIT(16)
#define BIT_DMAC_CHAN_PRIO(x)               (((x) & 0x3) << 1)
#define BIT_DMAC_CHAN_EN                    BIT(0)

/* DMAC_CHAN_SUSPEND */
#define BIT_DMAC_CHAN_SUSPEND_STS           BIT(16)
#define BIT_DMAC_CHAN_SUSPEND               BIT(0)

/* DMAC_CHAN_MSTCFG */
#define BIT_DMAC_CHAN_ARPROT                BIT(1)
#define BIT_DMAC_CHAN_AWPROT                BIT(0)


/*** dma channel description register bits define ***/
/* REG_DMAC_DMAC_SAR_0 */
#define BIT_DMAC_SRC_ADDR(x)                (((x) & 0xffffffff))

/* REG_DMAC_DMAC_SAR_1 */
#define BIT_DMAC_WB_UPPER_POINT(x)          (((x) & 0xffffffff))

/* REG_DMAC_DMAC_DAR_0 */
#define BIT_DMAC_DST_ADDR(x)                (((x) & 0xffffffff))

/* REG_DMAC_DMAC_DAR_1 */
#define BIT_DMAC_WB_LOWER_POINT(x)          (((x) & 0xffffffff))

/* REG_DMAC_DMAC_LCR_0 */
#define DMAC_BLOCK_LEN_MASK                 0xffff
#define DMAC_MBLOCK_LEN_MASK                0xffff
#define BIT_DMAC_MBLK_LENGTH(x)             (((x) & 0xffff) << 16)
#define BIT_DMAC_BLK_LENGTH(x)              (((x) & 0xffff))

/* REG_DMAC_DMAC_LCR_1 */
#define DMAC_FRAME_LEN_MASK                 0xfffffff
#define BIT_DMAC_REQ_STATUS                 BIT(31)
#define BIT_DMAC_REQ_MODE(x)                (((x) & 0x3) << 29)
#define BIT_DMAC_FRAME_LENGTH(x)            (((x) & 0xfffffff))

/* REG_DMAC_DMAC_SSR_0 */
#define DMAC_TRANS_STRIDE_MASK              0xffff
#define DMAC_BLOCK_STRIDE_MASK              0xffff
#define BIT_DMAC_SRC_BLK_STRIDE(x)          (((x) & 0xffff) << 16)
#define BIT_DMAC_SRC_TRANS_STRIDE(x)        (((x) & 0xffff))

/* REG_DMAC_DMAC_SSR_1 */
#define DMAC_MBLOCK_STRIDE_MASK             0xfffffff
#define BIT_DMAC_SRC_DATA_SIZE(x)           (((x) & 0x3) << 28)
#define BIT_DMAC_SRC_MBLK_STRIDE(x)         (((x) & 0xfffffff))

/* REG_DMAC_DMAC_DSR_0 */
#define BIT_DMAC_DST_BLK_STRIDE(x)          (((x) & 0xffff) << 16)
#define BIT_DMAC_DST_TRANS_STRIDE(x)        (((x) & 0xffff))

/* REG_DMAC_DMAC_DSR_1 */
#define BIT_DMAC_DST_DATA_SIZE(x)           (((x) & 0x3) << 28)
#define BIT_DMAC_DST_MBLK_STRIDE(x)         (((x) & 0xfffffff))

/* REG_DMAC_DMAC_LL_0 */
#define BIT_DMAC_LLP(x)                     (((x) & 0xffffffff))

/* REG_DMAC_DMAC_LL_1 */
//#define BIT_DMAC_BLK_LENGTH_STATUS(x)       (((x) & 0x1ffff) << 15)
//#define BIT_DMAC_SWAP_MODE(x)               (((x) & 0x3) << 13)
//#define BIT_DMAC_DST_TRANS_TYPE(x)          (((x) & 0x3) << 11)
//#define BIT_DMAC_SRC_TRANS_TYPE(x)          (((x) & 0x3) << 9)
#define BIT_DMAC_BLK_LENGTH_STATUS(x)       (((x) & 0x1ffff) << 16)
#define BIT_DMAC_DST_TRANS_TYPE(x)          (((x) & 0x3) << 14)
#define BIT_DMAC_SRC_TRANS_TYPE(x)          (((x) & 0x3) << 12)
#define BIT_DMAC_SWAP_MODE(x)               (((x) & 0x3) << 10)
#define BIT_DMAC_LLT_INTE                   BIT(8)
#define BIT_DMAC_LLT_FINTE                  BIT(7)
#define BIT_DMAC_LLLT_MINTE                 BIT(6)
#define BIT_DMAC_LLT_BINTE                  BIT(5)
#define BIT_DMAC_LLT_INT_OFFSET             0x5
#define BIT_DMAC_LLT_END                    BIT(4)
#define BIT_DMAC_LLT_END_OFFSET             0x4

#endif /* __DMA_REG_H__ */

