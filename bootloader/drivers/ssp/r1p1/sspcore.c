/*****************************************************************************/
/*                                                                           */
/*         Copyright 2021 by AIC Corporation. All rights reserved.           */
/*                                                                           */
/*****************************************************************************/

#include <aic_errno.h>
#include <aic_io.h>
#include <assert.h>
#include <cache.h>
#include <core_op.h>
#include <dma.h>
#include <gpio_inc.h>
#include <initcall.h>
#include <pin_ctrl.h>
#include <ssp.h>
#include <ssp/ssp_config.h>
#include <timer.h>
#include <stdlib.h>

#include "sspcore.h"

#define SSP_TX      (0x1 << 8)
#define SSP_RX      (0x1 << 9)
#define SSP_DMA_TX  (0x1 << 10)
#define SSP_DMA_RX  (0x1 << 11)

#define SSP_WAIT_FOREVER    (0xFFFFFFFF)

static LIST_HEAD(s_ssp_bus_list);

static void ssp_udelay(u32 usec)
{
    aic_udelay(usec);
}

static u32 ssp_get_utime(void)
{
    return aic_get_timestamp();
}

void *ssp_dma_malloc(u32 size)
{
    void *mem_ptr, *dma_mem_ptr;
    u8 *temp, offset;
    u32 alloc_size, cacheline_size;

    cacheline_size = get_dcache_linesize();

    /* to makesure allocated memory and actual size are both cacheline-aligned, 
     * we allocate (size + 2 * cacheline_size + 2) totally,
     * 2 for save offset & flag */
    alloc_size = size + 2 * cacheline_size + 2;

    mem_ptr = malloc(alloc_size);
    if (!mem_ptr)
        return NULL;

    /* get the cacheline-aligned address */
    dma_mem_ptr = (void *)ALIGN((u32)mem_ptr, cacheline_size);

    /* save offset from mem_ptr to act_mem_ptr */
    offset = (u32)dma_mem_ptr - (u32)mem_ptr;
    if (offset < 2) {
        /* reseve space to save offset */
        dma_mem_ptr = (void *)((u32)dma_mem_ptr + cacheline_size);
        offset = cacheline_size;
    }

    /* save align offset */
    temp = (u8 *)dma_mem_ptr - 1;
    *temp = 0xff;

    /* save align flag */
    temp = (u8 *)dma_mem_ptr - 2;
    *temp = offset;

    /* return the cacheline-aligned address */
    return dma_mem_ptr;
}

u8 *ssp_get_dynamic_real_ptr(u8 *mem_ptr)
{
    u8 *real_ptr, offset;

    /* get the offset from mem_ptr to dma_mem_ptr */
    offset = *(mem_ptr - 2);

    /* get the actual memory allocated from heap */
    real_ptr = mem_ptr - offset;

    return real_ptr;
}

void ssp_dma_free(u8 *dma_mem_ptr)
{
    u8 *mem_ptr;

    mem_ptr = ssp_get_dynamic_real_ptr(dma_mem_ptr);

    /* free memory */
    free(mem_ptr);
}

static void ssp_enable(u32 enable_reg, boolean en)
{
    if (en)
        AIC_WRITE32(enable_reg, 1);
    else
        AIC_WRITE32(enable_reg, 0);
}

static void ssp_dma_enable(struct ssp_reg *reg_base, boolean en)
{
    u32 reg_val;

    reg_val = AIC_READ32(&(reg_base->cr2));
    if (en)
        reg_val &= ~CR2_DMA_CLOSE;
    else
        reg_val |= CR2_DMA_CLOSE;
    AIC_WRITE32(&(reg_base->cr2), reg_val);
}

static void ssp_slave_reset(struct ssp_reg *reg_base)
{
    AIC_WRITE32(&(reg_base->soft_rst), 1);
    ssp_udelay(100);
    AIC_WRITE32(&(reg_base->soft_rst), 0);
    ssp_udelay(100);
}

static void ssp_start(struct ssp_reg *reg_base, boolean en)
{
    if (en)
        AIC_WRITE32(&(reg_base->start), 1);
    else
        AIC_WRITE32(&(reg_base->start), 0);

    ssp_udelay(30);
}

static void ssp_sfrm_default_val_set(struct ssp_reg *reg_base,
                                        enum active_lvl active)
{
    u32 reg_val;

    reg_val = AIC_READ32(&(reg_base->cr0));
    if (active == ACTIVE_LOW)
        reg_val |= CR0_SFRM_DEFAULT_VAL;
    else
        reg_val &= ~CR0_SFRM_DEFAULT_VAL;
    AIC_WRITE32(&(reg_base->cr0), reg_val);
}

static void ssp_src_clock_set(u32 clk_reg, u32 clk)
{
    u32 ref_sel = 0;

    if (clk == SSP_CLK_DPLL0_96M)
        ref_sel = 0;
    else if (clk == SSP_CLK_26M)
        ref_sel = 1;
    else if (clk == SSP_CLK_MSRC0_48M)
        ref_sel = 2;
    else if (clk == SSP_CLK_DPLL0_24P774M)
        ref_sel = 3;
    else if (clk == SSP_CLK_DPLL1_24P576M)
        ref_sel = 4;
    else
        ASSERT(0);

    AIC_WRITE32(clk_reg, ref_sel);
}

static void ssp_work_clock_set(struct ssp_ctrl *ctrl, u32 clk)
{
    struct ssp_reg *reg_base = (struct ssp_reg *)(ctrl->reg_base);
    u32 reg_val, div, remainder, quotient;

    if (clk >= (ctrl->src_clk / 2)) {
        div = 0;
    } else {
        remainder = ctrl->src_clk % clk;
        quotient = ctrl->src_clk / clk;
        if ((remainder == 0) && ((quotient & 0x1) == 0))
            div = ((ctrl->src_clk / clk) >> 1) - 1;
        else
            div = ((ctrl->src_clk / clk) & ~0x1) >> 1;
    }

    ASSERT(div <= 0xFF);

    reg_val = AIC_READ32(&(reg_base->cr0));
    reg_val &= ~CR0_BAUD_RATE_MASK;
    reg_val |= (div << CR0_BAUD_RATE_SHIFT) & CR0_BAUD_RATE_MASK;
    AIC_WRITE32(&(reg_base->cr0), reg_val);

    if (div == 0) {
        reg_val = AIC_READ32(&(reg_base->cr2));
        reg_val &= ~CR2_CAP_DATA_DLY_MASK;
        reg_val |= 0x1;
        AIC_WRITE32(&(reg_base->cr2), reg_val);
    }
}

static void ssp_ctrl_reset(u32 reset_reg)
{
    AIC_WRITE32(reset_reg, 0x1);
    ssp_udelay(300);
    AIC_WRITE32(reset_reg, 0x0);
    ssp_udelay(300);
}

static void ssp_toggle_enable(struct ssp_reg *reg_base, boolean en)
{
    u32 reg_val;

    reg_val = AIC_READ32(&(reg_base->cr1));
    if (en)
        reg_val |= CR1_CS_TOG_MODE;
    else
        reg_val &= ~CR1_CS_TOG_MODE;
    AIC_WRITE32(&(reg_base->cr1), reg_val);
}

static void ssp_bus_proto_set(struct ssp_reg *reg_base, enum bus_proto proto)
{
    u32 reg_val;

    reg_val = AIC_READ32(&(reg_base->cr0));

    /* proto clear */
    reg_val &= ~(CR0_I2S_DSP_MODE | CR0_I2S_MODE | CR0_SSP_MODE | CR0_MW_MODE);

    switch (proto) {
    case PROTO_SSP:
        reg_val |= CR0_SSP_MODE;
        break;
    case PROTO_MW:
        reg_val |= CR0_MW_MODE;
        break;
    case PROTO_I2S:
        reg_val |= CR0_I2S_MODE;
        break;
    case PROTO_I2S_DSP:
        reg_val |= (CR0_I2S_MODE | CR0_I2S_DSP_MODE);
        break;
    case PROTO_SPI:
        break;
    default:
        ASSERT(0);
        break;
    }

    AIC_WRITE32(&(reg_base->cr0), reg_val);
}

static void ssp_work_mode_set(struct ssp_reg *reg_base, enum work_mode mode)
{
    u32 reg_val;

    reg_val = AIC_READ32(&(reg_base->cr0));

    switch (mode) {
        case MASTER_MODE:
            reg_val |= CR0_MASTER_MODE;
            break;
        case SLAVE_MODE:
            reg_val &= ~(CR0_MASTER_MODE);
            break;
        default:
            ASSERT(0);
            break;
    }

    AIC_WRITE32(&(reg_base->cr0), reg_val);
}

static void ssp_duplex_mode_set(struct ssp_reg *reg_base, enum duplex_mode mode)
{
    u32 reg_val;

    reg_val = AIC_READ32(&(reg_base->cr0));

    switch (mode) {
    case FULL_DUPLEX:
        reg_val &= ~(CR0_HALF_DUPLEX_EN);
        break;
    case HALF_DUPLEX:
        reg_val |= CR0_HALF_DUPLEX_EN;
        break;
    default:
        ASSERT(0);
        break;
    }

    AIC_WRITE32(&(reg_base->cr0), reg_val);
}

static void ssp_spi_mode_set(struct ssp_reg *reg_base, enum spi_mode mode)
{
    u32 reg_val;

    reg_val = AIC_READ32(&(reg_base->cr0));

    switch (mode) {
    case SPI_MODE_0:
        /* CPOL = 0, CPHA = 0 */
        reg_val &= ~(CR0_CPOL | CR0_CPHA);
        break;
    case SPI_MODE_1:
        /* CPOL = 0, CPHA = 1 */
        reg_val &= ~(CR0_CPOL | CR0_CPHA);
        reg_val |= CR0_CPHA;
        break;
    case SPI_MODE_2:
        /* CPOL = 1, CPHA = 0 */
        reg_val &= ~(CR0_CPOL | CR0_CPHA);
        reg_val |= CR0_CPOL;
        break;
    case SPI_MODE_3:
        /* CPOL = 1, CPHA = 1 */
        reg_val &= ~(CR0_CPOL | CR0_CPHA);
        reg_val |= (CR0_CPOL | CR0_CPHA);
        break;
    default:
        ASSERT(0);
        break;
    }

    AIC_WRITE32(&(reg_base->cr0), reg_val);
}

static void ssp_data_width_set(struct ssp_reg *reg_base, enum data_width width)
{
    u32 reg_val;

    reg_val = AIC_READ32(&(reg_base->cr0));

    switch (width) {
        case DW_BYTE:
            /* set data beat: 0: 1x32bit; 1: 2x16bit; 3: 4x8bit */
            reg_val &= ~(CR0_DATA_BEAT_MASK);
            reg_val |= (0x0 << CR0_DATA_BEAT_SHIFT);

            /* set valid data bits: (N+1)bits */
            reg_val &= ~(CR0_DATA_LEN_MASK);
            reg_val |= (0x7 << CR0_DATA_LEN_SHIFT);

            /* CS width */
            reg_val &= ~(CR0_SFRM_WIDTH_MASK);
            reg_val |= (0x7 << CR0_SFRM_WIDTH_SHIFT);
            break;
        case DW_HWORD:
            /* set data beat: 0: 1x32bit; 1: 2x16bit; 3: 4x8bit */
            reg_val &= ~(CR0_DATA_BEAT_MASK);
            reg_val |= (0x0 << CR0_DATA_BEAT_SHIFT);

            /* set valid data bits: (N+1)bits */
            reg_val &= ~(CR0_DATA_LEN_MASK);
            reg_val |= (0xF << CR0_DATA_LEN_SHIFT);

            /* CS width */
            reg_val &= ~(CR0_SFRM_WIDTH_MASK);
            reg_val |= (0xF << CR0_SFRM_WIDTH_SHIFT);
            break;
        case DW_WORD:
            /* set data beat: 0: 1x32bit; 1: 2x16bit; 3: 4x8bit */
            reg_val &= ~(CR0_DATA_BEAT_MASK);
            reg_val |= (0x0 << CR0_DATA_BEAT_SHIFT);

            /* set valid data bits: (N+1)bits */
            reg_val &= ~(CR0_DATA_LEN_MASK);
            reg_val |= (0x1F << CR0_DATA_LEN_SHIFT);

            /* CS width */
            reg_val &= ~(CR0_SFRM_WIDTH_MASK);
            reg_val |= (0x1F << CR0_SFRM_WIDTH_SHIFT);
            break;
        default:
            ASSERT(0);
            break;
    }

    AIC_WRITE32(&(reg_base->cr0), reg_val);
}

static u32 ssp_get_tx_fifo_cnt(struct ssp_reg *reg_base)
{
    return (AIC_READ32(&(reg_base->sts0)) & STS_TX_FIFO_CNT_MASK) >>
            STS_TX_FIFO_CNT_SHIFT;
}

static u32 ssp_get_rx_fifo_cnt(struct ssp_reg *reg_base)
{
    return (AIC_READ32(&(reg_base->sts0)) & STS_RX_FIFO_CNT_MASK) >>
            STS_RX_FIFO_CNT_SHIFT;
}

static void ssp_set_tx_fifo_thrd(struct ssp_reg *reg_base, u8 thrd_val)
{
    u32 reg_val;

    /* set tx fifo watermark */
    reg_val = AIC_READ32(&(reg_base->cr2));
    reg_val &= ~(CR2_TXFIFO_THRD_MASK);
    reg_val |= (thrd_val << CR2_TXFIFO_THRD_SHIFT) & CR2_TXFIFO_THRD_MASK;
    AIC_WRITE32(&(reg_base->cr2), reg_val);
}

static void ssp_set_rx_fifo_thrd(struct ssp_reg *reg_base, u8 thrd_val)
{
    u32 reg_val;

    /* set rx fifo watermark */
    reg_val = AIC_READ32(&(reg_base->cr2));
    reg_val &= ~(CR2_RXFIFO_THRD_MASK);
    reg_val |= (thrd_val << CR2_RXFIFO_THRD_SHIFT) & CR2_RXFIFO_THRD_MASK;
    AIC_WRITE32(&(reg_base->cr2), reg_val);
}

static u32 ssp_get_bits_per_frame(struct ssp_reg *reg_base)
{
    return ((AIC_READ32(&(reg_base->cr0)) & CR0_SFRM_WIDTH_MASK) >>
            CR0_SFRM_WIDTH_SHIFT) + 1;
}

static u32 ssp_data_bytes_to_write(u8 *buf, u32 bytesperwrite, bool isbyteswap)
{
    u32 datatowrite = 0;

    switch (bytesperwrite)
    {
        case 1:
            datatowrite = *buf;
            ++buf;
            break;
        case 2:
            if (!isbyteswap) {
                datatowrite = *buf;
                ++buf;
                datatowrite |= *buf << 8;
                ++buf;
            } else {
                datatowrite = *buf << 8;
                ++buf;
                datatowrite |= *buf;
                ++buf;
            }
            break;
        case 4:
            if (!isbyteswap) {
                datatowrite = *buf;
                ++buf;
                datatowrite |= *buf << 8;
                ++buf;
                datatowrite |= *buf << 16;
                ++buf;
                datatowrite |= *buf << 24;
                ++buf;
            } else {
                datatowrite = *buf << 24;
                ++buf;
                datatowrite |= *buf << 16;
                ++buf;
                datatowrite |= *buf << 8;
                ++buf;
                datatowrite |= *buf;
                ++buf;
            }
            break;
        default:
            ASSERT(0);
            break;
    }

    return datatowrite;
}

static void ssp_data_bytes_to_read(u8 *buf, u32 readdata, u32 bytesperread,
                                    bool isbyteswap)
{
    switch (bytesperread)
    {
        case 1:
            *buf = readdata & 0xFF;
            ++buf;
            break;
        case 2:
            if (!isbyteswap) {
                *buf = readdata & 0xFF;
                ++buf;
                *buf = (readdata >> 8) & 0xFF;
                ++buf;
            } else {
                *buf = (readdata >> 8) & 0xFF;
                ++buf;
                *buf = readdata & 0xFF;
                ++buf;
            }
            break;
        case 4:
            if (!isbyteswap) {
                *buf = readdata & 0xFF;
                ++buf;
                *buf = (readdata >> 8) & 0xFF;
                ++buf;
                *buf = (readdata >> 16) & 0xFF;
                ++buf;
                *buf = (readdata >> 24) & 0xFF;
                ++buf;
            } else {
                *buf = (readdata >> 24) & 0xFF;
                ++buf;
                *buf = (readdata >> 16) & 0xFF;
                ++buf;
                *buf = (readdata >> 8) & 0xFF;
                ++buf;
                *buf = readdata & 0xFF;
                ++buf;
            }
            break;
        default:
            ASSERT(0);
            break;
    }
}

static void ssp_interrupt_enable(struct ssp_reg *reg_base, u32 int_map)
{
    u32 reg_val;

    reg_val = AIC_READ32(&(reg_base->int_msk));
    reg_val &= ~(int_map & INT_ALL);
    AIC_WRITE32(&(reg_base->int_msk), reg_val);
}

static void ssp_interrupt_disable(struct ssp_reg *reg_base, u32 int_map)
{
    u32 reg_val;

    reg_val = AIC_READ32(&(reg_base->int_msk));
    reg_val |= (int_map & INT_ALL);
    AIC_WRITE32(&(reg_base->int_msk), reg_val);
}

static void ssp_interrupt_clear(struct ssp_reg *reg_base, u32 int_map)
{
    u32 reg_val;

    reg_val = AIC_READ32(&(reg_base->int_clr));
    reg_val |= (int_map & INT_ALL);
    AIC_WRITE32(&(reg_base->int_clr), reg_val);
}

static u32 ssp_get_interrupt_status(struct ssp_reg *reg_base)
{
    return AIC_READ32(&(reg_base->int_sts));
}

static boolean ssp_is_bus_idle(struct ssp_reg *reg_base)
{
    u32 bus_sts = (AIC_READ32(&(reg_base->sts0)) & STS_BUS_STATUS_MASK)
                    >> STS_BUS_STATUS_SHIFT;
    return (bus_sts == 1 ? TRUE : FALSE);
}

static int ssp_polling_interrupt_status(struct ssp_reg *reg_base, u32 int_map,
                                        u32 timeout)
{
    u32 start_time, current_time;
    int ret = AIC_SUCCESS;

    start_time = ssp_get_utime();

    while ((ssp_get_interrupt_status(reg_base) & int_map) == 0) {
        if (timeout != SSP_WAIT_FOREVER) {
            current_time = ssp_get_utime();

            if ((current_time - start_time) > timeout) {
                ret = AIC_ETIMEDOUT;
                break;
            }
        }
    }

    ssp_interrupt_disable(reg_base, int_map);
    ssp_interrupt_clear(reg_base, int_map);

    return ret;
}

static int ssp_master_data_tx_rx(struct aic_ssp *ssp, u8 *tx_buf, u8 *rx_buf,
                                    u32 len)
{
    struct ssp_reg *reg_base = (struct ssp_reg *)(ssp->ctrl->reg_base);
    u32 tx_data, rx_data, txd_cnt, rxd_cnt, tx_cnt, rx_cnt;
    u32 bytespertrans = ssp_get_bits_per_frame(reg_base)/8;
    u32 int_map, i;
    int ret;

    ssp_duplex_mode_set(reg_base, FULL_DUPLEX);

    ssp_set_tx_fifo_thrd(reg_base, 0);
    ssp_set_rx_fifo_thrd(reg_base, SSP_FIFO_SIZE / 2);

    /* clear raw interrupts */
    ssp_interrupt_clear(reg_base, INT_ALL);

    int_map = INT_SSP_FINISH;
    txd_cnt = rxd_cnt = len / bytespertrans;

    while (txd_cnt > 0) {
        /* fill up tx fifo */
        tx_cnt = SSP_FIFO_SIZE - ssp_get_tx_fifo_cnt(reg_base);
        if (txd_cnt < tx_cnt)
            tx_cnt = txd_cnt;

        for (i = 0; i < tx_cnt; i++) {
            tx_data = ssp_data_bytes_to_write(tx_buf, bytespertrans, TRUE);
            AIC_WRITE32(&(reg_base->fifo), tx_data);
            tx_buf += bytespertrans;
        }

        ssp_interrupt_enable(reg_base, int_map);

        ssp_start(reg_base, TRUE);

        /* wait for data transfer complete */
        ret = ssp_polling_interrupt_status(reg_base, int_map, SSP_WAIT_FOREVER);
        ASSERT(ret == AIC_SUCCESS);

        ssp_start(reg_base, FALSE);

        /* read rx fifo */
        rx_cnt = 0;
        while (ssp_get_rx_fifo_cnt(reg_base) != 0) {
            rx_data = AIC_READ32(&(reg_base->fifo));
            ssp_data_bytes_to_read(rx_buf, rx_data, bytespertrans, TRUE);
            rx_buf += bytespertrans;
            rx_cnt++;
        }

        rxd_cnt -= rx_cnt;
        txd_cnt -= tx_cnt;
    }

    while ((rxd_cnt > 0) && (ssp_get_rx_fifo_cnt(reg_base) != 0)) {
        rx_data = AIC_READ32(&(reg_base->fifo));
        ssp_data_bytes_to_read(rx_buf, rx_data, bytespertrans, TRUE);
        rx_buf += bytespertrans;
        rxd_cnt--;
    }

    return AIC_SUCCESS;
}

static int ssp_master_data_tx(struct aic_ssp *ssp, u8 *tx_buf, u32 len)
{
    struct ssp_reg *reg_base = (struct ssp_reg *)(ssp->ctrl->reg_base);
    u32 tx_data, cnt, tx_cnt;
    u32 bytespertrans = ssp_get_bits_per_frame(reg_base)/8;
    u32 int_map, i;
    int ret;

    ssp_duplex_mode_set(reg_base, HALF_DUPLEX);

    ssp_set_tx_fifo_thrd(reg_base, 0);
    ssp_set_rx_fifo_thrd(reg_base, SSP_FIFO_SIZE / 2);

    /* clear raw interrupts */
    ssp_interrupt_clear(reg_base, INT_ALL);

    int_map = INT_SSP_FINISH;
    cnt = len / bytespertrans;

    while (cnt > 0) {
        /* fill up tx fifo */
        tx_cnt = SSP_FIFO_SIZE - ssp_get_tx_fifo_cnt(reg_base);
        if (cnt < tx_cnt)
            tx_cnt = cnt;

        for (i = 0; i < tx_cnt; i++) {
            tx_data = ssp_data_bytes_to_write(tx_buf, bytespertrans, TRUE);
            AIC_WRITE32(&(reg_base->fifo), tx_data);
            tx_buf += bytespertrans;
        }

        ssp_interrupt_enable(reg_base, int_map);

        ssp_start(reg_base, TRUE);

        /* wait for data transfer complete */
        ret = ssp_polling_interrupt_status(reg_base, int_map, SSP_WAIT_FOREVER);
        ASSERT(ret == AIC_SUCCESS);

        ssp_start(reg_base, FALSE);
        cnt -= tx_cnt;
    }

    return AIC_SUCCESS;
}

static int ssp_master_data_rx(struct aic_ssp *ssp, u8 *rx_buf, u32 len)
{
    struct ssp_reg *reg_base = (struct ssp_reg *)(ssp->ctrl->reg_base);
    u32 tx_dummy_data = 0, rx_data, cnt, tx_dummy_cnt, rx_cnt;
    u32 bytespertrans = ssp_get_bits_per_frame(reg_base)/8;
    u32 int_map, i;
    int rx_len, ret;
    u8 *rx_ptr = rx_buf;

    ssp_duplex_mode_set(reg_base, FULL_DUPLEX);

    ssp_set_tx_fifo_thrd(reg_base, 0);
    ssp_set_rx_fifo_thrd(reg_base, SSP_FIFO_SIZE / 2);

    /* clear raw interrupts */
    ssp_interrupt_clear(reg_base, INT_ALL);

    int_map = INT_SSP_FINISH;
    cnt = len / bytespertrans;

    while (cnt > 0) {
        /* fill up tx fifo */
        tx_dummy_cnt = SSP_FIFO_SIZE - ssp_get_rx_fifo_cnt(reg_base);
        if (cnt < tx_dummy_cnt)
            tx_dummy_cnt = cnt;

        for (i = 0; i < tx_dummy_cnt; i++)
            AIC_WRITE32(&(reg_base->fifo), tx_dummy_data);

        ssp_interrupt_enable(reg_base, int_map);

        ssp_start(reg_base, TRUE);

        /* wait for data transfer complete */
        ret = ssp_polling_interrupt_status(reg_base, int_map, SSP_WAIT_FOREVER);
        ASSERT(ret == AIC_SUCCESS);

        ssp_start(reg_base, FALSE);

        /* read rx fifo */
        rx_cnt = ssp_get_rx_fifo_cnt(reg_base);
        for (i = 0; i < rx_cnt; i++) {
            rx_data = AIC_READ32(&(reg_base->fifo));
            ssp_data_bytes_to_read(rx_ptr, rx_data, bytespertrans, TRUE);
            rx_ptr += bytespertrans;
        }

        cnt -= rx_cnt;
    }

    rx_len = (u32)rx_ptr - (u32)rx_buf;

    return rx_len;
}

static int ssp_slave_data_tx_rx(struct aic_ssp *ssp, u8 *tx_buf, u8 *rx_buf,
                                    u32 len)
{
    struct ssp_reg *reg_base = (struct ssp_reg *)(ssp->ctrl->reg_base);
    u32 tx_data, rx_data, txd_cnt, rxd_cnt, tx_cnt, rx_cnt;
    u32 bytespertrans = ssp_get_bits_per_frame(reg_base)/8;
    u32 int_map, i;
    int ret;

    ssp_slave_reset(reg_base);

    ssp_duplex_mode_set(reg_base, FULL_DUPLEX);

    ssp_set_rx_fifo_thrd(reg_base, SSP_FIFO_SIZE / 2);

    /* clear raw interrupts */
    ssp_interrupt_clear(reg_base, INT_ALL);

    int_map = INT_TXFIFO_BELOW_THRD;
    txd_cnt = rxd_cnt = len / bytespertrans;

    /* fill up tx fifo */
    tx_cnt = SSP_FIFO_SIZE - ssp_get_tx_fifo_cnt(reg_base);
    if (txd_cnt <= tx_cnt) {
        tx_cnt = txd_cnt;
        ssp_set_tx_fifo_thrd(reg_base, 0);
    } else {
        ssp_set_tx_fifo_thrd(reg_base, SSP_FIFO_SIZE / 2);
    }

    for (i = 0; i < tx_cnt; i++) {
        tx_data = ssp_data_bytes_to_write(tx_buf, bytespertrans, TRUE);
        AIC_WRITE32(&(reg_base->fifo), tx_data);
        tx_buf += bytespertrans;
    }

    ssp_interrupt_enable(reg_base, int_map);

    ssp_start(reg_base, TRUE);

    if (ssp->slv_tx_notify)
        ssp->slv_tx_notify(ssp->slv_param);

    /* wait for data transfer complete */
    ret = ssp_polling_interrupt_status(reg_base, int_map, SSP_WAIT_FOREVER);
    ASSERT(ret == AIC_SUCCESS);

    /* read rx fifo */
    rx_cnt = ssp_get_rx_fifo_cnt(reg_base);
    for (i = 0; i < rx_cnt; i++) {
        rx_data = AIC_READ32(&(reg_base->fifo));
        ssp_data_bytes_to_read(rx_buf, rx_data, bytespertrans, TRUE);
        rx_buf += bytespertrans;
    }

    txd_cnt -= tx_cnt;
    rxd_cnt -= rx_cnt;

    /* send and receive residual data */
    while (txd_cnt > 0) {
        tx_cnt = SSP_FIFO_SIZE - ssp_get_tx_fifo_cnt(reg_base);
        if (txd_cnt <= tx_cnt) {
            tx_cnt = txd_cnt;
            ssp_set_tx_fifo_thrd(reg_base, 0);
        } else {
            ssp_set_tx_fifo_thrd(reg_base, SSP_FIFO_SIZE / 2);
        }

        for (i = 0; i < tx_cnt; i++) {
            tx_data = ssp_data_bytes_to_write(tx_buf, bytespertrans, TRUE);
            AIC_WRITE32(&(reg_base->fifo), tx_data);
            tx_buf += bytespertrans;
        }

        ssp_interrupt_enable(reg_base, int_map);

        /* wait for data transfer complete */
        ret = ssp_polling_interrupt_status(reg_base, int_map, SSP_WAIT_FOREVER);
        ASSERT(ret == AIC_SUCCESS);

        /* read rx fifo */
        rx_cnt = ssp_get_rx_fifo_cnt(reg_base);
        for (i = 0; i < rx_cnt; i++) {
            rx_data = AIC_READ32(&(reg_base->fifo));
            ssp_data_bytes_to_read(rx_buf, rx_data, bytespertrans, TRUE);
            rx_buf += bytespertrans;
        }

        rxd_cnt -= rx_cnt;
        txd_cnt -= tx_cnt;
    }

    while ((rxd_cnt > 0) && (ssp_get_rx_fifo_cnt(reg_base) != 0)) {
        rx_data = AIC_READ32(&(reg_base->fifo));
        ssp_data_bytes_to_read(rx_buf, rx_data, bytespertrans, TRUE);
        rx_buf += bytespertrans;
        rxd_cnt--;
    }

    ssp_start(reg_base, FALSE);

    return AIC_SUCCESS;
}

static int ssp_slave_data_tx(struct aic_ssp *ssp, u8 *tx_buf, u32 len)
{
    struct ssp_reg *reg_base = (struct ssp_reg *)(ssp->ctrl->reg_base);
    u32 tx_data, cnt, tx_cnt, rx_dummy_data;
    u32 bytespertrans = ssp_get_bits_per_frame(reg_base)/8;
    u32 int_map, i;
    int ret;

    ssp_slave_reset(reg_base);

    ssp_duplex_mode_set(reg_base, FULL_DUPLEX);

    ssp_set_tx_fifo_thrd(reg_base, 0);
    ssp_set_rx_fifo_thrd(reg_base, SSP_FIFO_SIZE / 2);

    /* clear raw interrupts */
    ssp_interrupt_clear(reg_base, INT_ALL);

    int_map = INT_TXFIFO_BELOW_THRD;
    cnt = len / bytespertrans;

    /* fill up tx fifo */
    tx_cnt = SSP_FIFO_SIZE - ssp_get_tx_fifo_cnt(reg_base);
    if (cnt < tx_cnt) {
        ssp_set_tx_fifo_thrd(reg_base, 0);
        tx_cnt = cnt;
    } else {
        ssp_set_tx_fifo_thrd(reg_base, SSP_FIFO_SIZE / 2);
    }

    for (i = 0; i < tx_cnt; i++) {
        tx_data = ssp_data_bytes_to_write(tx_buf, bytespertrans, TRUE);
        AIC_WRITE32(&(reg_base->fifo), tx_data);
        tx_buf += bytespertrans;
    }

    ssp_interrupt_enable(reg_base, int_map);

    ssp_start(reg_base, TRUE);

    if (ssp->slv_tx_notify)
        ssp->slv_tx_notify(ssp->slv_param);

    /* wait for data transfer */
    ret = ssp_polling_interrupt_status(reg_base, int_map, SSP_WAIT_FOREVER);
    ASSERT(ret == AIC_SUCCESS);

    cnt -= tx_cnt;

    /* send residual data */
    while (cnt > 0) {
        tx_cnt = SSP_FIFO_SIZE - ssp_get_tx_fifo_cnt(reg_base);
        if (cnt < tx_cnt) {
            ssp_set_tx_fifo_thrd(reg_base, 0);
            tx_cnt = cnt;
        } else {
            ssp_set_tx_fifo_thrd(reg_base, SSP_FIFO_SIZE / 2);
        }

        for (i = 0; i < tx_cnt; i++) {
            tx_data = ssp_data_bytes_to_write(tx_buf, bytespertrans, TRUE);
            AIC_WRITE32(&(reg_base->fifo), tx_data);
            tx_buf += bytespertrans;
        }

        ssp_interrupt_enable(reg_base, int_map);

        /* wait for data transfer */
        ret = ssp_polling_interrupt_status(reg_base, int_map, SSP_WAIT_FOREVER);
        ASSERT(ret == AIC_SUCCESS);

        cnt -= tx_cnt;
    }

    /* clear rx fifo */
    while (ssp_get_rx_fifo_cnt(reg_base) != 0)
        rx_dummy_data = AIC_READ32(&(reg_base->fifo));

    /* make sure data really send */
    ssp_udelay(100);

    ssp_start(reg_base, FALSE);

    return AIC_SUCCESS;
}

static int ssp_slave_data_rx(struct aic_ssp *ssp, u8 *rx_buf, u32 len)
{
    struct ssp_reg *reg_base = (struct ssp_reg *)(ssp->ctrl->reg_base);
    u32 rx_data, cnt, rx_cnt;
    u32 bytespertrans = ssp_get_bits_per_frame(reg_base)/8;
    u32 int_map, i, to = 3000;
    int rx_len;
    u8 *rx_ptr = rx_buf;
    int ret;

    ssp_slave_reset(reg_base);

    ssp_duplex_mode_set(reg_base, HALF_DUPLEX);

    ssp_set_tx_fifo_thrd(reg_base, 0);

    /* clear raw interrupts */
    ssp_interrupt_clear(reg_base, INT_ALL);

    int_map = INT_RXFIFO_OVER_THRD;
    cnt = len / bytespertrans;

    if (cnt > SSP_FIFO_SIZE)
        ssp_set_rx_fifo_thrd(reg_base, SSP_FIFO_SIZE / 4);
    else
        ssp_set_rx_fifo_thrd(reg_base, cnt);

    ssp_interrupt_enable(reg_base, int_map);

    ssp_start(reg_base, TRUE);

    if (ssp->slv_rx_notify)
        ssp->slv_rx_notify(ssp->slv_param);
    else
        to = SSP_WAIT_FOREVER;

    /* wait for received data */
    ret = ssp_polling_interrupt_status(reg_base, int_map, to);

    /* read rx fifo */
    rx_cnt = ssp_get_rx_fifo_cnt(reg_base);
    if (cnt < rx_cnt)
        rx_cnt = cnt;

    for (i = 0; i < rx_cnt; i++) {
        rx_data = AIC_READ32(&(reg_base->fifo));
        ssp_data_bytes_to_read(rx_ptr, rx_data, bytespertrans, TRUE);
        rx_ptr += bytespertrans;
    }

    if (ret == AIC_ETIMEDOUT) {
        rx_len = (u32)rx_ptr - (u32)rx_buf;

        ssp_start(reg_base, FALSE);

        return rx_len;
    }

    cnt -= rx_cnt;

    while (cnt > 0) {
        if (cnt < (SSP_FIFO_SIZE / 2))
            ssp_set_rx_fifo_thrd(reg_base, cnt);

        ssp_interrupt_enable(reg_base, int_map);

        /* wait for data received data */
        ret = ssp_polling_interrupt_status(reg_base, int_map, to);

        /* read rx fifo */
        rx_cnt = ssp_get_rx_fifo_cnt(reg_base);
        for (i = 0; i < rx_cnt; i++) {
            rx_data = AIC_READ32(&(reg_base->fifo));
            ssp_data_bytes_to_read(rx_ptr, rx_data, bytespertrans, TRUE);
            rx_ptr += bytespertrans;
        }

        if (ret == AIC_ETIMEDOUT)
            break;

        cnt -= rx_cnt;
    }

    rx_len = (u32)rx_ptr - (u32)rx_buf;

    ssp_start(reg_base, FALSE);

    return rx_len;
}

static void ssp_data_width_switch(struct aic_ssp *ssp, u32 tx_base,
                                        u32 rx_base, u32 len)
{
    struct ssp_ctrl *ctrl = ssp->ctrl;
    struct ssp_reg *reg_base = (struct ssp_reg *)(ctrl->reg_base);

    if ((tx_base & 0x3) == 0 && (rx_base & 0x3) == 0 && (len & 0x3) == 0)
        ssp_data_width_set(reg_base, DW_WORD);
    else if ((tx_base & 0x1) == 0 && (rx_base & 0x1) == 0 && (len & 0x1) == 0)
        ssp_data_width_set(reg_base, DW_HWORD);
    else
        ssp_data_width_set(reg_base, DW_BYTE);
}

static int ssp_master_data_dma_tx_rx(struct aic_ssp *ssp, u8 *tx_buf,
                                        u8 *rx_buf, u32 len)
{
    struct ssp_ctrl *ctrl = ssp->ctrl;
    struct ssp_reg *reg_base = (struct ssp_reg *)(ctrl->reg_base);
    struct dma_chn_cfg dma_cfg;
    DMA_DATA_SIZE dma_tx_src_ds, dma_rx_src_ds, dma_tx_dst_ds, dma_rx_dst_ds;
    DMA_SWAP_MODE dma_swap_mode = DMA_ABCD;
    DMA_INT_TYPE dma_int_type;
    u32 dma_blk_len, cnt;
    u32 bytespertrans;
    int tx_chn, rx_chn;

    ASSERT(ssp_get_tx_fifo_cnt(reg_base) == 0);
    ASSERT(ssp_get_rx_fifo_cnt(reg_base) == 0);

    /* clear raw interrupts */
    ssp_interrupt_clear(reg_base, INT_ALL);

    ssp_duplex_mode_set(reg_base, FULL_DUPLEX);

    if (((u32)tx_buf & 0x3) == 0 && ((u32)rx_buf & 0x3) == 0 &&
            (len & 0x3) == 0) {
        ssp_data_width_set(reg_base, DW_WORD);
        dma_tx_dst_ds = DMA_WORD_SIZE;
        dma_tx_src_ds = DMA_WORD_SIZE;
        dma_rx_dst_ds = DMA_WORD_SIZE;
        dma_rx_src_ds = DMA_WORD_SIZE;
    } else if (((u32)tx_buf & 0x1) == 0 && ((u32)rx_buf & 0x1) == 0 &&
                (len & 0x1) == 0) {
        ssp_data_width_set(reg_base, DW_HWORD);
        dma_tx_dst_ds = DMA_HWORD_SIZE;
        dma_tx_src_ds = DMA_HWORD_SIZE;
        dma_rx_dst_ds = DMA_HWORD_SIZE;
        dma_rx_src_ds = DMA_HWORD_SIZE;
    } else {
        ssp_data_width_set(reg_base, DW_BYTE);
        dma_tx_dst_ds = DMA_BYTE_SIZE;
        dma_tx_src_ds = DMA_BYTE_SIZE;
        dma_rx_dst_ds = DMA_BYTE_SIZE;
        dma_rx_src_ds = DMA_BYTE_SIZE;
    }

    bytespertrans = ssp_get_bits_per_frame(reg_base)/8;
    switch (bytespertrans) {
        case 1:
            dma_swap_mode = DMA_ABCD;
            break;
        case 2:
            dma_swap_mode = DMA_BADC;
            break;
        case 4:
            dma_swap_mode = DMA_DCBA;
            break;
        default:
            ASSERT(0);
            break;
    }

    cnt = len / bytespertrans;

    /* SSP DMA TX config */
    tx_chn = aic_dma_chn_request(NULL, NULL);
    ASSERT(tx_chn >= 0);
    dma_int_type = DMA_NONE_INT;

    ssp_set_tx_fifo_thrd(reg_base, (SSP_FIFO_SIZE/2));
    dma_blk_len = bytespertrans;
    memset(&dma_cfg, 0, sizeof(struct dma_chn_cfg));
    dma_cfg.src_addr   = (u32)tx_buf;
    dma_cfg.dst_addr   = (u32)(&(reg_base->fifo));
    dma_cfg.blk_len    = dma_blk_len;
    dma_cfg.mblk_len   = len;
    dma_cfg.frame_len  = len;
    dma_cfg.src_ds     = dma_tx_src_ds;
    dma_cfg.dst_ds     = dma_tx_dst_ds;
    dma_cfg.trsf_type  = DMA_MEM_TO_DEV;
    dma_cfg.swap       = dma_swap_mode;
    dma_cfg.int_type   = dma_int_type;
    dma_cfg.req        = DMA_BLOCK_REQ;
    dma_cfg.hwid       = ctrl->dma_tx_id;

    aic_dma_chn_cfg(tx_chn, &dma_cfg);
    aic_dma_chn_start(tx_chn);

    /* SSP DMA RX config */
    rx_chn = aic_dma_chn_request(NULL, NULL);
    ASSERT(rx_chn >= 0);
    dma_int_type = DMA_NONE_INT;

    ssp_set_rx_fifo_thrd(reg_base, 1);
    dma_blk_len = bytespertrans;
    memset(&dma_cfg, 0, sizeof(struct dma_chn_cfg));
    dma_cfg.src_addr   = (u32)(&(reg_base->fifo));
    dma_cfg.dst_addr   = (u32)rx_buf;
    dma_cfg.blk_len    = dma_blk_len;
    dma_cfg.mblk_len   = len;
    dma_cfg.frame_len  = len;
    dma_cfg.src_ds     = dma_rx_src_ds;
    dma_cfg.dst_ds     = dma_rx_dst_ds;
    dma_cfg.trsf_type  = DMA_DEV_TO_MEM;
    dma_cfg.swap       = dma_swap_mode;
    dma_cfg.int_type   = dma_int_type;
    dma_cfg.req        = DMA_BLOCK_REQ;
    dma_cfg.hwid       = ctrl->dma_rx_id;

    aic_dma_chn_cfg(rx_chn, &dma_cfg);
    aic_dma_chn_start(rx_chn);

    /* ssp start */
    ssp_start(reg_base, TRUE);

    /* wait for transfer complete */
    aic_dma_chn_polling_done(tx_chn);
    aic_dma_chn_polling_done(rx_chn);

    /* ssp stop */
    ssp_start(reg_base, FALSE);

    ssp_interrupt_clear(reg_base, INT_ALL);

    aic_dma_chn_free(tx_chn);
    aic_dma_chn_free(rx_chn);

    return AIC_SUCCESS;
}

static int ssp_master_data_dma_tx(struct aic_ssp *ssp, u8 *tx_buf, u32 len)
{
    struct ssp_ctrl *ctrl = ssp->ctrl;
    struct ssp_reg *reg_base = (struct ssp_reg *)(ctrl->reg_base);
    struct dma_chn_cfg dma_cfg;
    DMA_DATA_SIZE dma_tx_src_ds, dma_tx_dst_ds;
    DMA_SWAP_MODE dma_swap_mode = DMA_ABCD;
    DMA_INT_TYPE dma_int_type;
    u32 dma_blk_len, tx_thrd, cnt;
    u32 bytespertrans;
    int tx_chn;

    ASSERT(ssp_get_tx_fifo_cnt(reg_base) == 0);
    ASSERT(ssp_get_rx_fifo_cnt(reg_base) == 0);

    cnt = len / bytespertrans;

    ssp_interrupt_clear(reg_base, INT_ALL);

    ssp_duplex_mode_set(reg_base, HALF_DUPLEX);

    if (((u32)tx_buf & 0x3) == 0 && (len & 0x3) == 0) {
        ssp_data_width_set(reg_base, DW_WORD);
        dma_tx_dst_ds = DMA_WORD_SIZE;
        dma_tx_src_ds = DMA_WORD_SIZE;
    } else if (((u32)tx_buf & 0x1) == 0 && (len & 0x1) == 0) {
        ssp_data_width_set(reg_base, DW_HWORD);
        dma_tx_dst_ds = DMA_HWORD_SIZE;
        dma_tx_src_ds = DMA_HWORD_SIZE;
    } else {
        ssp_data_width_set(reg_base, DW_BYTE);
        dma_tx_dst_ds = DMA_BYTE_SIZE;
        dma_tx_src_ds = DMA_BYTE_SIZE;
    }

    bytespertrans = ssp_get_bits_per_frame(reg_base)/8;
    switch (bytespertrans) {
        case 1:
            dma_swap_mode = DMA_ABCD;
            break;
        case 2:
            dma_swap_mode = DMA_BADC;
            break;
        case 4:
            dma_swap_mode = DMA_DCBA;
            break;
        default:
            ASSERT(0);
            break;
    }

    cnt = len / bytespertrans;

    /* SSP DMA TX config */
    tx_chn = aic_dma_chn_request(NULL, NULL);
    ASSERT(tx_chn >= 0);
    dma_int_type = DMA_NONE_INT;

    if (cnt < SSP_FIFO_SIZE) {
        tx_thrd = SSP_FIFO_SIZE - cnt;
    } else {
        if (cnt % (SSP_FIFO_SIZE / 2))
            tx_thrd = SSP_FIFO_SIZE - (cnt % (SSP_FIFO_SIZE / 2));
        else
            tx_thrd = SSP_FIFO_SIZE / 2;
    }
    ssp_set_tx_fifo_thrd(reg_base, tx_thrd);
    dma_blk_len = (SSP_FIFO_SIZE - tx_thrd) * bytespertrans;
    memset(&dma_cfg, 0, sizeof(struct dma_chn_cfg));
    dma_cfg.src_addr   = (u32)tx_buf;
    dma_cfg.dst_addr   = (u32)(&(reg_base->fifo));
    dma_cfg.blk_len    = dma_blk_len;
    dma_cfg.mblk_len   = len;
    dma_cfg.frame_len  = len;
    dma_cfg.src_ds     = dma_tx_src_ds;
    dma_cfg.dst_ds     = dma_tx_dst_ds;
    dma_cfg.trsf_type  = DMA_MEM_TO_DEV;
    dma_cfg.swap       = dma_swap_mode;
    dma_cfg.int_type   = dma_int_type;
    dma_cfg.req        = DMA_BLOCK_REQ;
    dma_cfg.hwid       = ctrl->dma_tx_id;

    aic_dma_chn_cfg(tx_chn, &dma_cfg);
    aic_dma_chn_start(tx_chn);

    /* ssp start */
    ssp_start(reg_base, TRUE);

    /* wait for transfer complete */
    aic_dma_chn_polling_done(tx_chn);

    /* wait for tx fifo empty */
    while (ssp_get_tx_fifo_cnt(reg_base)) {}

    /* wait for bus idle */
    while (ssp_is_bus_idle(reg_base) == FALSE) {}

    ASSERT(ssp_get_tx_fifo_cnt(reg_base) == 0);

    /* ssp stop */
    ssp_start(reg_base, FALSE);

    aic_dma_chn_free(tx_chn);

    return AIC_SUCCESS;
}

static int ssp_master_data_dma_rx(struct aic_ssp *ssp, u8 *rx_buf, u32 len)
{
    struct ssp_ctrl *ctrl = ssp->ctrl;
    struct ssp_reg *reg_base = (struct ssp_reg *)(ctrl->reg_base);
    struct dma_chn_cfg dma_cfg;
    DMA_DATA_SIZE dma_tx_src_ds, dma_tx_dst_ds, dma_rx_src_ds, dma_rx_dst_ds;
    DMA_SWAP_MODE dma_swap_mode = DMA_ABCD;
    DMA_INT_TYPE dma_int_type;
    u32 dma_blk_len, cnt, rx_len;
    u32 bytespertrans;
    int tx_chn, rx_chn;
    u8 *tx_dummy_ptr;
    u32 dummy_len = ROUND_UP(len, CACHE_LINE_SIZE);

    ASSERT(ssp_get_tx_fifo_cnt(reg_base) == 0);
    ASSERT(ssp_get_rx_fifo_cnt(reg_base) == 0);

    /* clear raw interrupts */
    ssp_interrupt_clear(reg_base, INT_ALL);

    ssp_duplex_mode_set(reg_base, FULL_DUPLEX);

    tx_dummy_ptr = ssp_dma_malloc(dummy_len);
    ASSERT(tx_dummy_ptr != NULL);

    memset(tx_dummy_ptr, 0x0, dummy_len);
    dcache_invcc_by_range((u32)tx_dummy_ptr, dummy_len);

    if (((u32)rx_buf & 0x3) == 0 && (len & 0x3) == 0) {
        ssp_data_width_set(reg_base, DW_WORD);
        dma_tx_dst_ds = DMA_WORD_SIZE;
        dma_tx_src_ds = DMA_WORD_SIZE;
        dma_rx_dst_ds = DMA_WORD_SIZE;
        dma_rx_src_ds = DMA_WORD_SIZE;
    } else if (((u32)rx_buf & 0x1) == 0 && (len & 0x1) == 0) {
        ssp_data_width_set(reg_base, DW_HWORD);
        dma_tx_dst_ds = DMA_HWORD_SIZE;
        dma_tx_src_ds = DMA_HWORD_SIZE;
        dma_rx_dst_ds = DMA_HWORD_SIZE;
        dma_rx_src_ds = DMA_HWORD_SIZE;
    } else {
        ssp_data_width_set(reg_base, DW_BYTE);
        dma_tx_dst_ds = DMA_BYTE_SIZE;
        dma_tx_src_ds = DMA_BYTE_SIZE;
        dma_rx_dst_ds = DMA_BYTE_SIZE;
        dma_rx_src_ds = DMA_BYTE_SIZE;
    }

    bytespertrans = ssp_get_bits_per_frame(reg_base)/8;
    switch (bytespertrans) {
        case 1:
            dma_swap_mode = DMA_ABCD;
            break;
        case 2:
            dma_swap_mode = DMA_BADC;
            break;
        case 4:
            dma_swap_mode = DMA_DCBA;
            break;
        default:
            ASSERT(0);
            break;
    }

    cnt = len / bytespertrans;

    /* SSP DMA TX config */
    tx_chn = aic_dma_chn_request(NULL, NULL);
    ASSERT(tx_chn >= 0);
    dma_int_type = DMA_NONE_INT;

    ssp_set_tx_fifo_thrd(reg_base, (SSP_FIFO_SIZE/2));
    dma_blk_len = bytespertrans;
    memset(&dma_cfg, 0, sizeof(struct dma_chn_cfg));
    dma_cfg.src_addr   = (u32)tx_dummy_ptr;
    dma_cfg.dst_addr   = (u32)(&(reg_base->fifo));
    dma_cfg.blk_len    = dma_blk_len;
    dma_cfg.mblk_len   = len;
    dma_cfg.frame_len  = len;
    dma_cfg.src_ds     = dma_tx_src_ds;
    dma_cfg.dst_ds     = dma_tx_dst_ds;
    dma_cfg.trsf_type  = DMA_MEM_TO_DEV;
    dma_cfg.swap       = dma_swap_mode;
    dma_cfg.int_type   = dma_int_type;
    dma_cfg.req        = DMA_BLOCK_REQ;
    dma_cfg.hwid       = ctrl->dma_tx_id;

    aic_dma_chn_cfg(tx_chn, &dma_cfg);
    aic_dma_chn_start(tx_chn);

    /* SSP DMA RX config */
    rx_chn = aic_dma_chn_request(NULL, NULL);
    ASSERT(rx_chn >= 0);
    dma_int_type = DMA_NONE_INT;

    ssp_set_rx_fifo_thrd(reg_base, 1);
    dma_blk_len = bytespertrans;
    memset(&dma_cfg, 0, sizeof(struct dma_chn_cfg));
    dma_cfg.src_addr   = (u32)(&(reg_base->fifo));
    dma_cfg.dst_addr   = (u32)rx_buf;
    dma_cfg.blk_len    = dma_blk_len;
    dma_cfg.mblk_len   = len;
    dma_cfg.frame_len  = len;
    dma_cfg.src_ds     = dma_rx_src_ds;
    dma_cfg.dst_ds     = dma_rx_dst_ds;
    dma_cfg.trsf_type  = DMA_DEV_TO_MEM;
    dma_cfg.swap       = dma_swap_mode;
    dma_cfg.int_type   = dma_int_type;
    dma_cfg.req        = DMA_BLOCK_REQ;
    dma_cfg.hwid       = ctrl->dma_rx_id;

    aic_dma_chn_cfg(rx_chn, &dma_cfg);
    aic_dma_chn_start(rx_chn);

    /* ssp start */
    ssp_start(reg_base, TRUE);

    /* wait for transfer complete */
    aic_dma_chn_polling_done(tx_chn);
    aic_dma_chn_polling_done(rx_chn);

    ASSERT(ssp_get_tx_fifo_cnt(reg_base) == 0);
    ASSERT(ssp_get_rx_fifo_cnt(reg_base) == 0);

    rx_len = aic_dma_chn_get_dst(rx_chn) - (u32)rx_buf;

    /* ssp stop */
    ssp_start(reg_base, FALSE);

    ssp_interrupt_clear(reg_base, INT_ALL);

    aic_dma_chn_free(tx_chn);
    aic_dma_chn_free(rx_chn);

    ssp_dma_free(tx_dummy_ptr);

    return rx_len;
}

static int ssp_slave_data_dma_tx_rx(struct aic_ssp *ssp, u8 *tx_buf, u8 *rx_buf,
                                        u32 len)
{
    struct ssp_ctrl *ctrl = ssp->ctrl;
    struct ssp_reg *reg_base = (struct ssp_reg *)(ctrl->reg_base);
    struct dma_chn_cfg dma_cfg;
    DMA_DATA_SIZE dma_data_size = DMA_BYTE_SIZE;
    DMA_SWAP_MODE dma_swap_mode = DMA_ABCD;
    u32 dma_blk_len, tx_thrd, rx_thrd, cnt;
    u32 bytespertrans = ssp_get_bits_per_frame(reg_base)/8;
    int tx_chn = 0, rx_chn;

    cnt = len / bytespertrans;

    ssp_slave_reset(reg_base);

    ssp_duplex_mode_set(reg_base, FULL_DUPLEX);

    switch (bytespertrans) {
        case 1:
            dma_data_size = DMA_BYTE_SIZE;
            dma_swap_mode = DMA_ABCD;
            break;
        case 2:
            dma_data_size = DMA_HWORD_SIZE;
            dma_swap_mode = DMA_BADC;
            break;
        case 4:
            dma_data_size = DMA_DWORD_SIZE;
            dma_swap_mode = DMA_DCBA;
            break;
        default:
            ASSERT(0);
            break;
    }

    /* SSP DMA TX config */
    tx_chn = aic_dma_chn_request(NULL, NULL);
    ASSERT(tx_chn >= 0);

    if (cnt < SSP_FIFO_SIZE) {
        tx_thrd = SSP_FIFO_SIZE - cnt;
    } else {
        if (cnt % (SSP_FIFO_SIZE / 2))
            tx_thrd = SSP_FIFO_SIZE - 1;
        else
            tx_thrd = SSP_FIFO_SIZE / 2;
    }
    ssp_set_tx_fifo_thrd(reg_base, tx_thrd);
    dma_blk_len = (SSP_FIFO_SIZE - tx_thrd) * bytespertrans;
    memset(&dma_cfg, 0, sizeof(struct dma_chn_cfg));
    dma_cfg.src_addr   = (u32)tx_buf;
    dma_cfg.dst_addr   = (u32)(&(reg_base->fifo));
    dma_cfg.blk_len    = dma_blk_len;
    dma_cfg.mblk_len   = len;
    dma_cfg.frame_len  = len;
    dma_cfg.src_ds     = dma_data_size;
    dma_cfg.dst_ds     = dma_data_size;
    dma_cfg.trsf_type  = DMA_MEM_TO_DEV;
    dma_cfg.swap       = dma_swap_mode;
    dma_cfg.int_type   = DMA_NONE_INT;
    dma_cfg.req        = DMA_BLOCK_REQ;
    dma_cfg.hwid       = ctrl->dma_tx_id;

    aic_dma_chn_cfg(tx_chn, &dma_cfg);
    aic_dma_chn_start(tx_chn);

    /* SSP DMA RX config */
    rx_chn = aic_dma_chn_request(NULL, NULL);
    ASSERT(rx_chn >= 0);

    if (cnt < SSP_FIFO_SIZE) {
        rx_thrd = cnt;
    } else {
        if (cnt % (SSP_FIFO_SIZE / 2))
            rx_thrd = 1;
        else
            rx_thrd = SSP_FIFO_SIZE / 2;
    }
    ssp_set_rx_fifo_thrd(reg_base, rx_thrd);
    dma_blk_len = rx_thrd  * bytespertrans;
    memset(&dma_cfg, 0, sizeof(struct dma_chn_cfg));
    dma_cfg.src_addr   = (u32)(&(reg_base->fifo));
    dma_cfg.dst_addr   = (u32)rx_buf;
    dma_cfg.blk_len    = dma_blk_len;
    dma_cfg.mblk_len   = len;
    dma_cfg.frame_len  = len;
    dma_cfg.src_ds     = dma_data_size;
    dma_cfg.dst_ds     = dma_data_size;
    dma_cfg.trsf_type  = DMA_DEV_TO_MEM;
    dma_cfg.swap       = dma_swap_mode;
    dma_cfg.int_type   = DMA_NONE_INT;
    dma_cfg.req        = DMA_BLOCK_REQ;
    dma_cfg.hwid       = ctrl->dma_rx_id;

    aic_dma_chn_cfg(rx_chn, &dma_cfg);
    aic_dma_chn_start(rx_chn);

    /* ssp start */
    ssp_start(reg_base, TRUE);

    if (ssp->slv_tx_notify)
        ssp->slv_tx_notify(ssp->slv_param);

    /* wait for transfer complete */
    aic_dma_chn_polling_done(tx_chn);
    aic_dma_chn_polling_done(rx_chn);

    /* ssp stop */
    ssp_start(reg_base, FALSE);

    ssp_interrupt_clear(reg_base, INT_ALL);

    aic_dma_chn_free(tx_chn);
    aic_dma_chn_free(rx_chn);

    return AIC_SUCCESS;
}

static int ssp_slave_data_dma_tx(struct aic_ssp *ssp, u8 *tx_buf, u32 len)
{
    struct ssp_ctrl *ctrl = ssp->ctrl;
    struct ssp_reg *reg_base = (struct ssp_reg *)(ctrl->reg_base);
    struct dma_chn_cfg dma_cfg;
    DMA_DATA_SIZE dma_data_size = DMA_BYTE_SIZE;
    DMA_SWAP_MODE dma_swap_mode = DMA_ABCD;
    u32 dma_blk_len, tx_thrd, rx_thrd, cnt;
    u32 bytespertrans = ssp_get_bits_per_frame(reg_base)/8;
    int tx_chn = 0, rx_chn;
    u8 *rx_dummy_ptr;
    u32 dummy_len = ROUND_UP(len, CACHE_LINE_SIZE);

    rx_dummy_ptr = ssp_dma_malloc(dummy_len);
    ASSERT(rx_dummy_ptr != NULL);

    dcache_inv_by_range((u32)rx_dummy_ptr, dummy_len);

    cnt = len / bytespertrans;

    ssp_slave_reset(reg_base);

    ssp_duplex_mode_set(reg_base, FULL_DUPLEX);

    switch (bytespertrans) {
        case 1:
            dma_data_size = DMA_BYTE_SIZE;
            dma_swap_mode = DMA_ABCD;
            break;
        case 2:
            dma_data_size = DMA_HWORD_SIZE;
            dma_swap_mode = DMA_BADC;
            break;
        case 4:
            dma_data_size = DMA_DWORD_SIZE;
            dma_swap_mode = DMA_DCBA;
            break;
        default:
            ASSERT(0);
            break;
    }

    /* SSP DMA TX config */
    tx_chn = aic_dma_chn_request(NULL, NULL);
    ASSERT(tx_chn >= 0);

    if (cnt < SSP_FIFO_SIZE) {
        tx_thrd = SSP_FIFO_SIZE - cnt;
    } else {
        if (cnt % (SSP_FIFO_SIZE / 2))
            tx_thrd = SSP_FIFO_SIZE - 1;
        else
            tx_thrd = SSP_FIFO_SIZE / 2;
    }
    ssp_set_tx_fifo_thrd(reg_base, tx_thrd);
    dma_blk_len = (SSP_FIFO_SIZE - tx_thrd) * bytespertrans;
    memset(&dma_cfg, 0, sizeof(struct dma_chn_cfg));
    dma_cfg.src_addr   = (u32)tx_buf;
    dma_cfg.dst_addr   = (u32)(&(reg_base->fifo));
    dma_cfg.blk_len    = dma_blk_len;
    dma_cfg.mblk_len   = len;
    dma_cfg.frame_len  = len;
    dma_cfg.src_ds     = dma_data_size;
    dma_cfg.dst_ds     = dma_data_size;
    dma_cfg.trsf_type  = DMA_MEM_TO_DEV;
    dma_cfg.swap       = dma_swap_mode;
    dma_cfg.int_type   = DMA_NONE_INT;
    dma_cfg.req        = DMA_BLOCK_REQ;
    dma_cfg.hwid       = ctrl->dma_tx_id;

    aic_dma_chn_cfg(tx_chn, &dma_cfg);
    aic_dma_chn_start(tx_chn);

    /* SSP DMA RX config */
    rx_chn = aic_dma_chn_request(NULL, NULL);
    ASSERT(rx_chn >= 0);

    if (cnt < SSP_FIFO_SIZE) {
        rx_thrd = cnt;
    } else {
        if (cnt % (SSP_FIFO_SIZE / 2))
            rx_thrd = 1;
        else
            rx_thrd = SSP_FIFO_SIZE / 2;
    }
    ssp_set_rx_fifo_thrd(reg_base, rx_thrd);
    dma_blk_len = rx_thrd  * bytespertrans;
    memset(&dma_cfg, 0, sizeof(struct dma_chn_cfg));
    dma_cfg.src_addr   = (u32)(&(reg_base->fifo));
    dma_cfg.dst_addr   = (u32)rx_dummy_ptr;
    dma_cfg.blk_len    = dma_blk_len;
    dma_cfg.mblk_len   = len;
    dma_cfg.frame_len  = len;
    dma_cfg.src_ds     = dma_data_size;
    dma_cfg.dst_ds     = dma_data_size;
    dma_cfg.trsf_type  = DMA_DEV_TO_MEM;
    dma_cfg.swap       = dma_swap_mode;
    dma_cfg.int_type   = DMA_NONE_INT;
    dma_cfg.req        = DMA_BLOCK_REQ;
    dma_cfg.hwid       = ctrl->dma_rx_id;

    aic_dma_chn_cfg(rx_chn, &dma_cfg);
    aic_dma_chn_start(rx_chn);

    /* ssp start */
    ssp_start(reg_base, TRUE);

    if (ssp->slv_tx_notify)
        ssp->slv_tx_notify(ssp->slv_param);

    /* wait for transfer complete */
    aic_dma_chn_polling_done(tx_chn);
    aic_dma_chn_polling_done(rx_chn);

    /* ssp stop */
    ssp_start(reg_base, FALSE);

    ssp_interrupt_clear(reg_base, INT_ALL);

    aic_dma_chn_free(tx_chn);
    aic_dma_chn_free(rx_chn);

    ssp_dma_free(rx_dummy_ptr);

    return AIC_SUCCESS;
}

static int ssp_slave_data_dma_rx(struct aic_ssp *ssp, u8 *rx_buf, u32 len)
{
    struct ssp_ctrl *ctrl = ssp->ctrl;
    struct ssp_reg *reg_base = (struct ssp_reg *)(ctrl->reg_base);
    struct dma_chn_cfg dma_cfg;
    DMA_DATA_SIZE dma_data_size = DMA_BYTE_SIZE;
    DMA_SWAP_MODE dma_swap_mode = DMA_ABCD;
    u32 dma_blk_len, rx_thrd, cnt, rx_data, i;
    u32 bytespertrans = ssp_get_bits_per_frame(reg_base)/8;
    int rx_chn, rx_len, ret;
    u32 to = 3000;
    u8 *rx_ptr = rx_buf;

    cnt = len / bytespertrans;

    ssp_slave_reset(reg_base);

    ssp_duplex_mode_set(reg_base, HALF_DUPLEX);

    switch (bytespertrans) {
        case 1:
            dma_data_size = DMA_BYTE_SIZE;
            dma_swap_mode = DMA_ABCD;
            break;
        case 2:
            dma_data_size = DMA_HWORD_SIZE;
            dma_swap_mode = DMA_BADC;
            break;
        case 4:
            dma_data_size = DMA_DWORD_SIZE;
            dma_swap_mode = DMA_DCBA;
            break;
        default:
            ASSERT(0);
            break;
    }

    /* SSP DMA RX config */
    rx_chn = aic_dma_chn_request(NULL, NULL);
    ASSERT(rx_chn >= 0);

    if (cnt < SSP_FIFO_SIZE) {
        rx_thrd = cnt;
    } else {
        if (cnt % (SSP_FIFO_SIZE / 2))
            rx_thrd = 1;
        else
            rx_thrd = SSP_FIFO_SIZE / 2;
    }
    ssp_set_rx_fifo_thrd(reg_base, rx_thrd);
    dma_blk_len = rx_thrd  * bytespertrans;
    memset(&dma_cfg, 0, sizeof(struct dma_chn_cfg));
    dma_cfg.src_addr   = (u32)(&(reg_base->fifo));
    dma_cfg.dst_addr   = (u32)rx_buf;
    dma_cfg.blk_len    = dma_blk_len;
    dma_cfg.mblk_len   = len;
    dma_cfg.frame_len  = len;
    dma_cfg.src_ds     = dma_data_size;
    dma_cfg.dst_ds     = dma_data_size;
    dma_cfg.trsf_type  = DMA_DEV_TO_MEM;
    dma_cfg.swap       = dma_swap_mode;
    dma_cfg.int_type   = DMA_NONE_INT;
    dma_cfg.req        = DMA_BLOCK_REQ;
    dma_cfg.hwid       = ctrl->dma_rx_id;

    aic_dma_chn_cfg(rx_chn, &dma_cfg);
    aic_dma_chn_start(rx_chn);

    /* ssp start */
    ssp_start(reg_base, TRUE);

    if (ssp->slv_rx_notify)
        ssp->slv_rx_notify(ssp->slv_param);
    else
        to = SSP_WAIT_FOREVER;

    /* wait for transfer complete */
    ret = aic_dma_chn_polling_timeout(rx_chn, to);
    ASSERT((ret == AIC_SUCCESS) || (ret == AIC_ETIMEDOUT));

    rx_len = aic_dma_chn_get_dst(rx_chn) - (u32)rx_buf;

    rx_ptr += rx_len;

    /* read residual data in rx fifo */
    cnt = ssp_get_rx_fifo_cnt(reg_base);
    for (i = 0; i < cnt; i++) {
        rx_data = AIC_READ32(&(reg_base->fifo));
        ssp_data_bytes_to_read(rx_ptr, rx_data, bytespertrans, TRUE);
        rx_ptr += bytespertrans;
        rx_len += bytespertrans;
    }

    /* ssp stop */
    ssp_start(reg_base, FALSE);

    ssp_interrupt_clear(reg_base, INT_ALL);

    aic_dma_chn_free(rx_chn);

    return rx_len;
}

static int ssp_master_data_transfer(struct ssp_device *dev,
                                    struct ssp_transfer *xfer)
{
    struct aic_ssp *ssp = dev->bus;
    int ret;

    if (ssp->ctrl->dma_en) {
        if (xfer->tx_buf && xfer->rx_buf) {
            dcache_invcc_by_range((u32)(xfer->tx_buf), xfer->length);
            dcache_invcc_by_range((u32)(xfer->rx_buf), xfer->length);
            ret = ssp_master_data_dma_tx_rx(ssp, xfer->tx_buf, xfer->rx_buf,
                                             xfer->length);
        } else if (xfer->tx_buf && !(xfer->rx_buf)) {
            dcache_invcc_by_range((u32)(xfer->tx_buf), xfer->length);
            ret = ssp_master_data_dma_tx(ssp, xfer->tx_buf, xfer->length);
        } else if (!(xfer->tx_buf) && xfer->rx_buf) {
            dcache_invcc_by_range((u32)(xfer->rx_buf), xfer->length);
            ret = ssp_master_data_dma_rx(ssp, xfer->rx_buf, xfer->length);
        } else
            ret = AIC_EINVAL;
    } else {
        if (xfer->tx_buf && xfer->rx_buf) {
            ssp_data_width_switch(ssp, (u32)(xfer->tx_buf),
                                    (u32)(xfer->rx_buf), xfer->length);
            ret = ssp_master_data_tx_rx(ssp, xfer->tx_buf, xfer->rx_buf,
                                             xfer->length);
        } else if (xfer->tx_buf && !(xfer->rx_buf)) {
            ssp_data_width_switch(ssp, (u32)(xfer->tx_buf), 0, xfer->length);
            ret = ssp_master_data_tx(ssp, xfer->tx_buf, xfer->length);
        } else if (!(xfer->tx_buf) && xfer->rx_buf) {
            ssp_data_width_switch(ssp, 0, (u32)(xfer->rx_buf), xfer->length);
            ret = ssp_master_data_rx(ssp, xfer->rx_buf, xfer->length);
        } else
            ret = AIC_EINVAL;
    }

    return ret;
}

static int ssp_slave_data_transfer(struct ssp_device *dev,
                                    struct ssp_transfer *xfer)
{
    struct aic_ssp *ssp = dev->bus;
    int ret;

    if (ssp->ctrl->dma_en) {
        if (xfer->tx_buf && xfer->rx_buf) {
            ssp_data_width_switch(ssp, (u32)(xfer->tx_buf),
                                    (u32)(xfer->rx_buf), xfer->length);
            dcache_invcc_by_range((u32)(xfer->tx_buf), xfer->length);
            dcache_invcc_by_range((u32)(xfer->rx_buf), xfer->length);
            ret = ssp_slave_data_dma_tx_rx(ssp, xfer->tx_buf, xfer->rx_buf,
                                            xfer->length);
        } else if (xfer->tx_buf && !(xfer->rx_buf)) {
            ssp_data_width_switch(ssp, (u32)(xfer->tx_buf), 0, xfer->length);
            dcache_invcc_by_range((u32)(xfer->tx_buf), xfer->length);
            ret = ssp_slave_data_dma_tx(ssp, xfer->tx_buf, xfer->length);
        } else if (!(xfer->tx_buf) && xfer->rx_buf) {
            ssp_data_width_switch(ssp, 0, (u32)(xfer->rx_buf), xfer->length);
            dcache_invcc_by_range((u32)(xfer->rx_buf), xfer->length);
            ret = ssp_slave_data_dma_rx(ssp, xfer->rx_buf, xfer->length);
        } else
            ret = AIC_EINVAL;
    } else {
        if (xfer->tx_buf && xfer->rx_buf) {
            ssp_data_width_switch(ssp, (u32)(xfer->tx_buf),
                                    (u32)(xfer->rx_buf), xfer->length);
            ret = ssp_slave_data_tx_rx(ssp, xfer->tx_buf, xfer->rx_buf,
                                            xfer->length);
        } else if (xfer->tx_buf && !(xfer->rx_buf)) {
            ssp_data_width_switch(ssp, (u32)(xfer->tx_buf), 0, xfer->length);
            ret = ssp_slave_data_tx(ssp, xfer->tx_buf, xfer->length);
        } else if (!(xfer->tx_buf) && xfer->rx_buf) {
            ssp_data_width_switch(ssp, 0, (u32)(xfer->rx_buf), xfer->length);
            ret = ssp_slave_data_rx(ssp, xfer->rx_buf, xfer->length);
        } else
            ret = AIC_EINVAL;
    }

    return ret;
}

void ssp_cs_set(struct ssp_device *dev, boolean enable)
{
    if (dev->cfg.cs == SSP_NO_CS || dev->auto_cs)
        return;

    if (dev->cs_active == ACTIVE_HIGH) {
        if (enable)
            aic_gpio_output(dev->cs_gpio, GPIO_OUTPUT_LVL_HIGH);
        else
            aic_gpio_output(dev->cs_gpio, GPIO_OUTPUT_LVL_LOW);
    } else {
        if (enable)
            aic_gpio_output(dev->cs_gpio, GPIO_OUTPUT_LVL_LOW);
        else
            aic_gpio_output(dev->cs_gpio, GPIO_OUTPUT_LVL_HIGH);
    }
}

int ssp_data_transfer(struct ssp_device *dev, struct ssp_message *msg)
{
    struct ssp_transfer *transfer;
    int ret = AIC_ENODEV;

    if (dev->bus->slave) {
        list_for_each_entry(transfer, &msg->head, node, __typeof(*transfer)) {
            ret = ssp_slave_data_transfer(dev, transfer);
            if (ret < 0)
                break;
        }
    } else {
        ssp_cs_set(dev, TRUE);

        list_for_each_entry(transfer, &msg->head, node, __typeof(*transfer)) {
            ret = ssp_master_data_transfer(dev, transfer);
            if (ret < 0)
                break;
        }

        ssp_cs_set(dev, FALSE);
    }

    return ret;
}

void ssp_controller_configure(struct ssp_device *dev, struct ssp_cfg *cfg)
{
    struct aic_ssp *ssp = dev->bus;
    struct ssp_ctrl *ctrl = ssp->ctrl;
    struct ssp_reg *reg_base = (struct ssp_reg *)(ctrl->reg_base);

    ssp_enable(ctrl->enable_reg, TRUE);

    ssp_ctrl_reset(ctrl->reset_reg);

    ssp_start(reg_base, FALSE);

    ssp_interrupt_disable(reg_base, INT_ALL);

    ssp_work_mode_set(reg_base, ctrl->work_mode);

    if (ctrl->work_mode == MASTER_MODE) {
        ssp_work_clock_set(ctrl, cfg->work_clk);

        if (cfg->cs != SSP_NO_CS) {
            ASSERT(cfg->cs < ctrl->cs_num);
            dev->cs_active = ctrl->cs_pin[cfg->cs].active;
            dev->cs_gpio = ctrl->cs_pin[cfg->cs].gpio_id;
            dev->auto_cs = ctrl->auto_cs;
            if (dev->auto_cs)
                ssp_sfrm_default_val_set(reg_base, dev->cs_active);
        } else {
            dev->cs_active = ACTIVE_LVL_MAX;
            dev->cs_gpio = GPIO_PIN_MAX;
            dev->auto_cs = FALSE;
        }
    } else if (ctrl->work_mode == SLAVE_MODE) {
        ssp->slave = TRUE;

        if (cfg->slv_tx_notify)
            ssp->slv_tx_notify = cfg->slv_tx_notify;

        if (cfg->slv_rx_notify)
            ssp->slv_rx_notify = cfg->slv_rx_notify;

        if (cfg->slv_param)
            ssp->slv_param = cfg->slv_param;
    } else {
        ASSERT(0);
    }

    ssp_bus_proto_set(reg_base, ctrl->bus_proto);

    if (ctrl->bus_proto == PROTO_SPI)
        ssp_spi_mode_set(reg_base, cfg->spi_mode);

    ssp_dma_enable(reg_base, ctrl->dma_en);

    ssp_toggle_enable(reg_base, TRUE);
}

struct aic_ssp *ssp_get_handle(char *name)
{
    struct aic_ssp *ssp = NULL;

    list_for_each_entry(ssp, &s_ssp_bus_list, node, __typeof(*ssp)) {
        if (ssp && (strcmp(ssp->ctrl->name, name) == 0))
            return ssp;
    }

    return NULL;
}

static struct ssp_ops s_ssp_ops = {
    .transfer = ssp_data_transfer,
    .ctrl_configure = ssp_controller_configure,
};

static int ssp_init(void)
{
    struct aic_ssp *ssp;

    ssp = (struct aic_ssp*)malloc(sizeof(*ssp));
    ASSERT(ssp != NULL);

    memset(ssp, 0x0, sizeof(*ssp));

    ssp->ctrl = ssp_get_ctrl();

    ssp->ops = &s_ssp_ops;

    ssp_src_clock_set(ssp->ctrl->clk_reg, ssp->ctrl->src_clk);

    /* cs init */
    if (ssp->ctrl->work_mode == MASTER_MODE) {
        int i, ret;
        for (i = 0; i < ssp->ctrl->cs_num; i++) {
            pin_func_config(ssp->ctrl->cs_pin[i].pin_reg,
                            ssp->ctrl->cs_pin[i].func);
            if (ssp->ctrl->auto_cs == FALSE) {
                ret = aic_gpio_request(ssp->ctrl->cs_pin[i].gpio_id);
                ASSERT(ret == AIC_SUCCESS);

                if (ssp->ctrl->cs_pin[i].active == ACTIVE_HIGH)
                    aic_gpio_output(ssp->ctrl->cs_pin[i].gpio_id,
                                    GPIO_OUTPUT_LVL_LOW);
                else
                    aic_gpio_output(ssp->ctrl->cs_pin[i].gpio_id,
                                    GPIO_OUTPUT_LVL_HIGH);
            }
        }
    }

    list_head_init(&(ssp->head));

    list_head_init(&(ssp->node));
    /* add ssp node to ssp bus list */
    list_add_tail(&ssp->node, &(s_ssp_bus_list));

    return 0;
}
service_init(ssp_init);
