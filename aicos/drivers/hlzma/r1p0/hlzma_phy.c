/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Physical driver of hardware lzma module.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x01b40c
#define LOG_MODULE    AMOD_DRV

#include <aic_type.h>
#include <aic_errno.h>
#include <aic_bitmap.h>
#include <aic_io.h>
#include <reg_base.h>
#include <aic_iot_clk.h>
#include <aic_clk_map.h>

#include <aic_reg_base.h>
#include "../hlzma_phy.h"

#define LZMA_REG_BASE                       REG_BASE_LZMA
#define REG_LZMA_STA0                       (LZMA_REG_BASE + 0x0)
#define REG_LZMA_STA1                       (LZMA_REG_BASE + 0x4)
#define REG_LZMA_STA2                       (LZMA_REG_BASE + 0x8)
#define REG_LZMA_STA3                       (LZMA_REG_BASE + 0xc)
#define REG_LZMA_START                      (LZMA_REG_BASE + 0x10)
#define REG_LZMA_PARA_INFO0                 (LZMA_REG_BASE + 0x14)
#define REG_LZMA_PROT                       (LZMA_REG_BASE + 0x18)
#define REG_LZMA_IN_ADDR                    (LZMA_REG_BASE + 0x20)
#define REG_LZMA_OUT_ADDR                   (LZMA_REG_BASE + 0x24)
#define REG_LZMA_INT_OUT                    (LZMA_REG_BASE + 0x30)
#define REG_LZMA_INT_RAW                    (LZMA_REG_BASE + 0x34)
#define REG_LZMA_INT_EN                     (LZMA_REG_BASE + 0x38)
#define REG_LZMA_INT_MASK                   (LZMA_REG_BASE + 0x3c)
#define REG_LZMA_INT_CLR                    (LZMA_REG_BASE + 0x40)
#define REG_LZMA_AXIM_CTRL                  (LZMA_REG_BASE + 0x44)
#define REG_LZMA_AXIM_STA0                  (LZMA_REG_BASE + 0x48)
#define REG_LZMA_AXIM_STA1                  (LZMA_REG_BASE + 0x4c)
#define REG_LZMA_AXIM_STA2                  (LZMA_REG_BASE + 0x50)
#define REG_LZMA_AXIM_STA3                  (LZMA_REG_BASE + 0x54)
#define REG_LZMA_FETCH_STA0                 (LZMA_REG_BASE + 0x60)
#define REG_LZMA_FETCH_STA1                 (LZMA_REG_BASE + 0x64)
#define REG_LZMA_STORE_STA0                 (LZMA_REG_BASE + 0x68)
#define REG_LZMA_STORE_STA1                 (LZMA_REG_BASE + 0x6c)
#define REG_LZMA_STORE_STA2                 (LZMA_REG_BASE + 0x70)
#define REG_LZMA_STORE_STA3                 (LZMA_REG_BASE + 0x74)
#define REG_LZMA_STORE_STA4                 (LZMA_REG_BASE + 0x78)
#define REG_LZMA_CORE_STA0                  (LZMA_REG_BASE + 0x80)
#define BIT_LZMA_EB                         BIT_0

/*REG_LZMA_STA0*/
#define BIT_STA0_MASK                       0x1f
#define BIT_STA0_DONE                       BIT(0)
/*REG_LZMA_START*/
#define BITS_START_MASK                     0xffff
#define LZMA_START_MAGIC                    0x123a

/*REG_LZMA_PROT*/
#define BIT_START_PROT                      BIT(0)

/*LZMA_REG_LZMA_PARA_INFO0*/
#define BIT_BLK_MEM_PARA0(x)                (((x) & 0x3ff) << 22)
#define BIT_BLK_MEM_WFIFO_THD(x)            (((x) & 0xfff) << 10)
#define BIT_BLK_MEM_RFIFO_THD(x)            (((x) & 0x3f) << 4)
#define BIT_BLK_MEM_WR_ENDIAN(x)            (((x) & 0x3) << 2)
#define BIT_BLK_MEM_RD_ENDIAN(x)            (((x) & 0x3))

/*REG_LZMA_INT_OUT*/
#define BIT_BLK_MEM_INT_OUT(x)              (((x) & 0xffff))

/*REG_LZMA_INT_RAW*/
#define BIT_BLK_MEM_INT_RAW(x)              (((x) & 0xffff))

/*REG_LZMA_INT_EN*/
#define BIT_BLK_MEM_INT_EN(x)               (((x) & 0xffff))

/*REG_LZMA_INT_MASK*/
#define BIT_BLK_MEM_INT_MASK(x)             (((x) & 0xffff))

/*REG_LZMA_INT_CLR*/
#define BIT_BLK_MEM_INT_CLR(x)              (((x) & 0xffff))

/*REG_LZMA_AXIM_CTRL*/
#define BIT_BLK_MEM_AXIM_INTERVAL_CYCLE(x)  (((x) & 0xff) << 24)
#define BIT_BLK_MEM_AXIM_R_OUTSTANDING(x)   (((x) & 0xff) << 16)
#define BIT_BLK_MEM_AXIM_W_OUTSTANDING(x)   (((x) & 0xff) << 8)
#define BIT_BLK_MEM_AXIM_TRANS_STOP         BIT(6)
#define BIT_BLK_MEM_AXIM_MAXLEN_8           BIT(5)
#define BIT_BLK_MEM_AXIM_DBG_EN             BIT(4)
#define BIT_BLK_MEM_GMI_RCHNL_BUSY          BIT(3)
#define BIT_BLK_MEM_GMI_WCHNL_BUSY          BIT(2)
#define BIT_BLK_MEM_AXI_RCHNL_BUSY          BIT(1)
#define BIT_BLK_MEM_AXI_WCHNL_BUSY          BIT(0)

/* Enable or disable hardware lzma */
void hlzma_phy_en(BOOL en)
{
    if (en) {
#ifdef CLK_CPU_AHB_LZMA_EN
        clk_enable(CLK_CPU_AHB_LZMA_EN);
#endif
#ifdef CLK_MCU_LZMA_EN
        clk_enable(CLK_MCU_LZMA_EN);
#endif
        AIC_WRITE32(REG_LZMA_EB, BIT_LZMA_EB);
#ifdef CLK_MCU_LZMA
        clk_set_parent_comp(CLK_MCU_LZMA, 1);
#endif
    } else {
        AIC_WRITE32(REG_LZMA_EB, ~BIT_LZMA_EB);
#ifdef CLK_MCU_LZMA_EN
        clk_disable(CLK_MCU_LZMA_EN);
#endif
#ifdef CLK_CPU_AHB_LZMA_EN
        clk_disable(CLK_CPU_AHB_LZMA_EN);
#endif
    }
}

/* start lzma decompress processing */
s32 hlzma_phy_start (void)
{
    /* disable protection */
    AIC_WRITE32(REG_LZMA_PROT, BIT_START_PROT);
    /* reset first */
    AIC_WRITE32(REG_LZMA_START, BITS_START_MASK&LZMA_START_MAGIC);
    /* enable protection */
    AIC_WRITE32(REG_LZMA_PROT, 0x0);
    return AIC_SUCCESS;
}

static void __delay(u32 ticks)
{
    volatile u32 i = ticks;

    while (i--) {}

}

/* reset lzma module */
s32 hlzma_phy_reset (void)
{
    AIC_WRITE32(REG_LZMA_RESET, 0x1);
    __delay(0x10);
    AIC_WRITE32(REG_LZMA_RESET, 0x0);

    return AIC_SUCCESS;
}

/* setting parameters */
s32 hlzma_phy_cfg(const hlzma_setting_t *cfg)
{
    AIC_WRITE32(REG_LZMA_IN_ADDR, cfg->in_addr);
    AIC_WRITE32(REG_LZMA_OUT_ADDR, cfg->out_addr);
    return AIC_SUCCESS;
}

/* getting done */
s32 hlzma_phy_polling_done(void)
{
    u32 status;

    while(!((status = AIC_READ32(REG_LZMA_STA0)) & BIT_STA0_MASK));

    if (status & BIT_STA0_DONE)
        return AIC_SUCCESS;
    else
        return AIC_EIO;
}

/* setting interrupt */
s32 hlzma_phy_int_cfg(void)
{
    /* clear */
    AIC_WRITE32(REG_LZMA_INT_CLR, BIT_DEC_DONE | BIT_DEC_ERR);
    /* unmask */
    AIC_WRITE32(REG_LZMA_INT_MASK, 0x0);
    /* en */
    AIC_WRITE32(REG_LZMA_INT_EN, BIT_DEC_DONE | BIT_DEC_ERR);

    return AIC_SUCCESS;
}

/* lzma interrupt handler */
void hlzma_int_handler(void *param)
{
    u32 sts;
    u32 *val = param;

    sts = AIC_READ32(REG_LZMA_INT_OUT);
    /* clear */
    AIC_WRITE32(REG_LZMA_INT_CLR, BIT_DEC_DONE | BIT_DEC_ERR);
    if (sts & BIT_DEC_DONE)
        *val = 0x7e7e7e7e;
    else
        *val = 0xdeaddead;
}

/* Disable all interrupt after reset as all int are enabled after reset */
void hlzma_phy_int_disable(void)
{
    AIC_WRITE32(REG_LZMA_INT_EN, 0x0);
    return;
}
