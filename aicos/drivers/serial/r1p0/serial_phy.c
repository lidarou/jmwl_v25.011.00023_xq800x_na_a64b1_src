/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0af190
#define LOG_MODULE    AMOD_DRV

#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_osa.h>
#include <irq.h>
#include <serial.h>
#include <serial/serial_config.h>
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif
#include <aic_assert.h>
#include <dma.h>
#include <cache.h>
#include <aic_math.h>
#include <timer_inc.h>
#include <misc.h>
#include <eipu.h>
#include <aic_iot_clk.h>
#include <aic_clk_map.h>

#define SERIAL_CTRL         0x00
#define SERIAL_MODE         0x04
#define SERIAL_IER          0x08
#define SERIAL_IDR          0x0c
#define SERIAL_IMR          0x10
#define SERIAL_CISR         0x14
#define SERIAL_BRGR         0x18
#define SERIAL_RTOR         0x1c
#define SERIAL_RTR          0x20
#define SERIAL_MCR          0x24
#define SERIAL_MSR          0x28
#define SERIAL_CSR          0x2c
#define SERIAL_RFIFO        0x30
#define SERIAL_TFIFO        0x30
#define SERIAL_BDIVR        0x34
#define SERIAL_FCDR         0x38
#define SERIAL_RPWR         0x3c
#define SERIAL_TPWR         0x40
#define SERIAL_TTR          0x44
#define SERIAL_RBSR         0x48
#define SERIAL_DMA_RFULL    0x4C
#define SERIAL_DMA_REMPTY   0x50
#define SERIAL_DMA_TFULL    0x54
#define SERIAL_DMA_TEMPTY   0x58

/* ctrl */
#define SERIAL_DMA_RXEN     BIT_10
#define SERIAL_DMA_TXEN     BIT_9
#define SERIAL_STPBRK       BIT_8
#define SERIAL_STTBRK       BIT_7
#define SERIAL_STTO         BIT_6
#define SERIAL_TXDIS        BIT_5
#define SERIAL_TXEN         BIT_4
#define SERIAL_RXDIS        BIT_3
#define SERIAL_RXEN         BIT_2
#define SERIAL_TXRES        BIT_1
#define SERIAL_RXRES        BIT_0

/* int enable */
#define SERIAL_INT_RBRK     BIT_13
#define SERIAL_INT_TOVR     BIT_12
#define SERIAL_INT_TNFUL    BIT_11
#define SERIAL_INT_TTRIG    BIT_10

/* csr */
#define SERIAL_CSR_RBRK     BIT_15
#define SERIAL_CSR_TNFUL    BIT_14
#define SERIAL_CSR_TTRIG    BIT_13
#define SERIAL_CSR_FDELT    BIT_12
#define SERIAL_CSR_TACTIVE  BIT_11
#define SERIAL_CSR_RACTIVE  BIT_10

/* common */
#define SERIAL_DMSI         BIT_9
#define SERIAL_TIMEOUT      BIT_8
#define SERIAL_PARE         BIT_7
#define SERIAL_FRAME        BIT_6
#define SERIAL_ROVR         BIT_5
#define SERIAL_TFUL         BIT_4
#define SERIAL_TEMPTY       BIT_3
#define SERIAL_RFUL         BIT_2
#define SERIAL_REMPTY       BIT_1
#define SERIAL_RTRIG        BIT_0

/* modem status */
#define SERIAL_FCTRL_STATUS BIT_8
#define SERIAL_DCD_STATUS   BIT_7
#define SERIAL_RI_STATUS    BIT_6
#define SERIAL_DSR_STATUS   BIT_5
#define SERIAL_CTS_STATUS   BIT_4

#define SERIAL_USED         1
#define SERIAL_UNUSED       0

#define SERIAL_FLOW_CTL_CFG(a)  ((a) << 5)
#define SERIAL_RTS_CFG(a)       ((a) << 1)
#define SERIAL_DTR_CFG(a)       ((a) << 0)
#define SERIAL_xTRIG(v)         ((v) & 0x3f)

#define SERIAL_FIFO_DEPTH       64

#define BAUD_SAMPLE(s,c) ((s) / (c))
#define BDIV(a,b) (((((a) * 10) / (b) + 5) / 10) - 1)

#define SERIAL_WAIT_TIME_CNT    100

#define SERIAL_REG_SIZE         0x80

static u8 aon_clk_index = 1;

struct list_head uart_cb_list;

static aos_thread_id serial_thread_id;
static aos_queue_ptr serial_thread_queue;

#define SERIAL_RECV_DATA_TIMEOUT    1000 /* unit:ms */

#define SERIAL_TASK_STACK_SIZE      200
#define SERIAL_TASK_PRIORITY        1
#define SERIAL_TASK_QUEUE_CNT       10

enum _serial_queue_status {
    SERIAL_SLEEP = 0,
    SERIAL_WAKEUP,
};
extern u32 slp_get_delaysleep(void);

void aic_uart_dma_uninit(struct uart_port *up);
void aic_uart_hw_uninit(struct aic_uart *auart);
void aic_uart_hw_init(struct aic_uart *auart);
void aic_uart_startup(struct uart_port *up, struct aic_uart *auart);
void aic_uart_dma_init(struct uart_port *up);

void aic_uart_rx_fctrl_level_set(struct aic_uart *auart, u32 val)
{
    auart->fcdr = val;
    AIC_WRITE32(auart->base + SERIAL_FCDR, val);
}

void aic_uart_tx_trigger_level_set(void *dev, u32 val)
{
    struct aic_uart *auart = (struct aic_uart *)dev;

    auart->ttr = SERIAL_xTRIG(val);
    AIC_WRITE32(auart->base + SERIAL_TTR, auart->ttr);
}

void aic_uart_tx_trigger_level_get(void *dev, u32 *val)
{
    struct aic_uart *auart = (struct aic_uart *)dev;

    *val = SERIAL_xTRIG(AIC_READ32(auart->base + SERIAL_TTR));
}

void aic_uart_rx_trigger_level_set(void *dev, u32 val)
{
    struct aic_uart *auart = (struct aic_uart *)dev;

    auart->rtr = SERIAL_xTRIG(val);
    AIC_WRITE32(auart->base + SERIAL_RTR, SERIAL_xTRIG(val));
}

void aic_uart_rx_trigger_level_get(void *dev, u32 *val)
{
    struct aic_uart *auart = (struct aic_uart *)dev;

    *val = SERIAL_xTRIG(AIC_READ32(auart->base + SERIAL_RTR));
}

void aic_uart_timeout_set(void *dev, u32 val)
{
    struct aic_uart *auart = (struct aic_uart *)dev;

    AIC_WRITE32(auart->base + SERIAL_RTOR, val);
}

int aic_uart_rx_fifo_triger_get(void *dev)
{
    struct aic_uart *auart = (struct aic_uart *)dev;

    return AIC_READ32(auart->base + SERIAL_CSR) & SERIAL_RTRIG;
}

int aic_uart_tx_fifo_triger_get(void *dev)
{
    struct aic_uart *auart = (struct aic_uart *)dev;

    return AIC_READ32(auart->base + SERIAL_CSR) & SERIAL_CSR_TTRIG;
}

void aic_uart_fctrl_set(void *dev, u32 val)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    u32 status;

    status = AIC_READ32(auart->base + SERIAL_MCR);
    status &= ~(SERIAL_FLOW_CTL_CFG(1));
    AIC_WRITE32(auart->base + SERIAL_MCR,
                status | SERIAL_FLOW_CTL_CFG(val & 0x1));
}

void aic_uart_rts_set(void *dev, u32 val)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    u32 status;

    status = AIC_READ32(auart->base + SERIAL_MCR);
    AIC_WRITE32(auart->base + SERIAL_MCR, status | SERIAL_RTS_CFG(val & 0x1));
}

void aic_uart_dtr_set(void *dev, u32 val)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    u32 status;

    status = AIC_READ32(auart->base + SERIAL_MCR);
    AIC_WRITE32(auart->base + SERIAL_MCR, status | SERIAL_DTR_CFG(val & 0x1));
}

void aic_uart_rpwr_set(void *dev)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
     u32 bdivr, brgr, val;

    bdivr = AIC_READ32(auart->base + SERIAL_BDIVR);
    brgr = AIC_READ32(auart->base + SERIAL_BRGR);

    val = (bdivr + 1) * brgr * 3 / 16;
    AIC_WRITE32(auart->base + SERIAL_RPWR, val);
}

void aic_uart_tpwr_set(void *dev)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    u32 bdivr, val;

    bdivr = AIC_READ32(auart->base + SERIAL_BDIVR);
    val = (bdivr + 1) * 3 / 16;
    AIC_WRITE32(auart->base + SERIAL_TPWR, val);
}

void aic_uart_wsize_set(void *dev, u32 val)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    union uart_mode u_mode;

    auart->wsize = val;
    u_mode.mode = AIC_READ32(auart->base + SERIAL_MODE);
    u_mode.mode_bitmap.WSIZE = auart->wsize;
    AIC_WRITE32(auart->base + SERIAL_MODE, u_mode.mode);
}

void aic_uart_mode_set(void *dev, u8 mode)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    union uart_mode u_mode;

    u_mode.mode = AIC_READ32(auart->base + SERIAL_MODE);
    u_mode.mode_bitmap.CHMODE = mode & 0x3;
    AIC_WRITE32(auart->base + SERIAL_MODE, u_mode.mode);
}

void aic_uart_nbstop_set(void *dev, u8 nbstop)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    union uart_mode u_mode;

    u_mode.mode = AIC_READ32(auart->base + SERIAL_MODE);
    u_mode.mode_bitmap.NBSTOP = nbstop & 0x3;
    AIC_WRITE32(auart->base + SERIAL_MODE, u_mode.mode);
}

void aic_uart_nbstop_get(void *dev, u8 *nbstop)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    union uart_mode u_mode;

    u_mode.mode = AIC_READ32(auart->base + SERIAL_MODE);
    *nbstop = u_mode.mode_bitmap.NBSTOP;
}

void aic_uart_par_set(void *dev, u8 par)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    union uart_mode u_mode;

    u_mode.mode = AIC_READ32(auart->base + SERIAL_MODE);
    u_mode.mode_bitmap.PAR = par & 0x7;
    AIC_WRITE32(auart->base + SERIAL_MODE, u_mode.mode);
}

void aic_uart_par_get(void *dev, u8 *par)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    union uart_mode u_mode;

    u_mode.mode = AIC_READ32(auart->base + SERIAL_MODE);
    *par = u_mode.mode_bitmap.PAR;
}

void aic_uart_chrl_set(void *dev, u8 chrl)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    union uart_mode u_mode;

    u_mode.mode = AIC_READ32(auart->base + SERIAL_MODE);
    u_mode.mode_bitmap.CHRL = chrl & 0x3;
    AIC_WRITE32(auart->base + SERIAL_MODE, u_mode.mode);
}

void aic_uart_chrl_get(void *dev, u8 *chrl)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    union uart_mode u_mode;

    u_mode.mode = AIC_READ32(auart->base + SERIAL_MODE);
    *chrl = u_mode.mode_bitmap.CHRL;
}

void aic_uart_iomux_set(u32 src_inf, u32 dst_inf)
{
    u32 mask, val;

    val = AIC_READ32(SERIAL_IOMUX_SEL);

    /* clear */
    mask = 0xF << (src_inf << 2);
    val = val & (~mask);

    /* set */
    val = val | (dst_inf << (src_inf << 2));
    AIC_WRITE32(SERIAL_IOMUX_SEL, val);
}

void aic_uart_iomux_sel(void *dev, u32 dst_inf)
{
    struct aic_uart *auart = (struct aic_uart *)dev;

    /* config src to dst */
    aic_uart_iomux_set(auart->inf, dst_inf);

    /* config dst to src */
    aic_uart_iomux_set(dst_inf, auart->inf);

    auart->dst_inf = dst_inf;
}

void aic_uart_baud_get(void *dev, u32 *val)
{
    struct aic_uart *auart = (struct aic_uart *)dev;

    *val = auart->baud;
}

void aic_uart_fctrl_get(void *dev, u32 *val)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    u32 status;

    status = AIC_READ32(auart->base + SERIAL_MSR);
    *val = (status & SERIAL_FCTRL_STATUS) >> 8;
}

void aic_uart_dcd_get(void *dev, u32 *val)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    u32 status;

    status = AIC_READ32(auart->base + SERIAL_MSR);
    *val = (status & SERIAL_DCD_STATUS) >> 0x7;
}

void aic_uart_ri_get(void *dev, u32 *val)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    u32 status;

    status = AIC_READ32(auart->base + SERIAL_MSR);
    *val = (status & SERIAL_RI_STATUS) >> 0x6;
}

void aic_uart_dsr_get(void *dev, u32 *val)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    u32 status;

    status = AIC_READ32(auart->base + SERIAL_MSR);
    *val = (status & SERIAL_DSR_STATUS) >> 0x5;
}

void aic_uart_cts_get(void *dev, u32 *val)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    u32 status;

    status = AIC_READ32(auart->base + SERIAL_MSR);
    *val = (status & SERIAL_CTS_STATUS) >> 0x4;
}

void aic_uart_wsize_get(void *dev, u32 *val)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    *val = auart->wsize;
}

size_t aic_uart_write(void *dev, const char *s, u32 n)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    u32 i, tsize;

    if (!auart->opencnt)
        return 0;

    switch (auart->wsize) {
        case SERIAL_WSIZE4:
            tsize = ALIGN_DOWN(n, 4);
            for (i = 0; i < tsize; i += 4) {
                AIC_WRITE32(auart->base + SERIAL_TFIFO, *(u32 *)(s + i));
            }
            break;
        case SERIAL_WSIZE2:
            tsize = ALIGN_DOWN(n, 2);
            for (i = 0; i < tsize; i += 2) {
                AIC_WRITE32(auart->base + SERIAL_TFIFO, *(u16 *)(s + i));
            }
            break;
        case SERIAL_WSIZE1:
        default:
            tsize = n;
            for (i = 0; i < tsize; i++) {
                AIC_WRITE32(auart->base + SERIAL_TFIFO, *(u8 *)(s + i));
            }
            break;
    }

    return tsize;
}

size_t aic_uart_read(void *dev, char *s, u32 n)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    u32 i = 0, triger_level, rsize, tsize;

    if (!auart->opencnt)
        return 0;

    switch (auart->wsize) {
        case SERIAL_WSIZE4:
            tsize = ALIGN_DOWN(n, 4);
            aic_uart_rx_trigger_level_get(auart, &triger_level);

            while ((tsize > 0) && !aic_uart_rx_fifo_empty(auart) &&
                     aic_uart_rx_fifo_triger_get(auart)) {
                rsize = triger_level > tsize ? tsize : triger_level;
                for (i = 0; i < rsize; i += 4) {
                    *(u32 *)(s + i) = AIC_READ32(auart->base + SERIAL_RFIFO);
                }
                s += rsize;
                tsize -= rsize;
            }
            i = n - tsize;
            break;
        case SERIAL_WSIZE2:
            tsize = ALIGN_DOWN(n, 2);
            aic_uart_rx_trigger_level_get(auart, &triger_level);

            while ((tsize > 0) && !aic_uart_rx_fifo_empty(auart) &&
                 aic_uart_rx_fifo_triger_get(auart)) {
                rsize = triger_level > tsize ? tsize : triger_level;
                for (i = 0; i < rsize; i += 2) {
                    *(u16 *)(s + i) = AIC_READ32(auart->base + SERIAL_RFIFO);
                }
                s += rsize;
                tsize -= rsize;
            }
            i = n - tsize;
            break;
        case SERIAL_WSIZE1:
        default:
            while ((i < n) && !aic_uart_rx_fifo_empty(auart)) {
                *(u8 *)(s + i) = AIC_READ32(auart->base + SERIAL_RFIFO);
                i++;
            }
            break;
    }

    return i;
}

void aic_uart_baud_set(void *dev, u32 baud)
{
    u32 bdiv, minbdiv, cd, baud_sample, i, tmp_bdiv;
    struct aic_uart *auart = (struct aic_uart *)dev;

    bdiv = auart->source_clk / baud;
    if ((bdiv >= 4) && (bdiv <= 0xFF)) {
        auart->cd = 1;
        baud_sample = BAUD_SAMPLE(auart->source_clk, auart->cd);
        bdiv = BDIV(baud_sample, baud);
    } else if (bdiv > 0xFF) {
        minbdiv = 4;
        cd = bdiv / minbdiv;
        while (cd > 0xFFFF) {
            cd = bdiv / ++minbdiv;
        }
        auart->cd = cd;
        baud_sample = BAUD_SAMPLE(auart->source_clk, auart->cd);
        bdiv = BDIV(baud_sample, baud);
    } else {
        auart->cd = 1;
        bdiv = 3;
    }

    for (i = 0; i < 100; i++) {
        AIC_WRITE32(auart->base + SERIAL_BRGR, auart->cd);
        AIC_WRITE32(auart->base + SERIAL_BDIVR, bdiv);

        cd = AIC_READ32(auart->base + SERIAL_BRGR);
        tmp_bdiv = AIC_READ32(auart->base + SERIAL_BDIVR);
        if (auart->cd == cd && bdiv == tmp_bdiv)
            break;
    }
    AIC_ASSERT(i < 100);

    auart->baud = baud;
}

int aic_uart_tx_fifo_ttriger(struct aic_uart *auart)
{
    return AIC_READ32(auart->base + SERIAL_CSR) & SERIAL_CSR_TTRIG;
}

int aic_uart_rx_fifo_empty(void *dev)
{
    struct aic_uart *auart = (struct aic_uart *)dev;

    return AIC_READ32(auart->base + SERIAL_CSR) & SERIAL_REMPTY;
}

int aic_uart_tx_fifo_empty(void *dev)
{
    struct aic_uart *auart = (struct aic_uart *)dev;

    return AIC_READ32(auart->base + SERIAL_CSR) & SERIAL_TEMPTY;
}

void aic_uart_dma_tx(void *dev, const char *s, u32 n)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    struct dma_chn_cfg dma_cfg = {0};
    DMA_DATA_SIZE ds;
    u32 ctrl;

    AOS_INTERRUPT_DECLARE

    ctrl = AIC_READ32(auart->base + SERIAL_CTRL);
    ctrl |= SERIAL_DMA_TXEN;
    AIC_WRITE32(auart->base + SERIAL_CTRL, ctrl);

    switch (auart->wsize) {
        case SERIAL_WSIZE4:
            ds = DMA_WORD_SIZE;
            break;
        case SERIAL_WSIZE2:
            ds = DMA_HWORD_SIZE;
            break;
        case SERIAL_WSIZE1:
        default:
            ds = DMA_BYTE_SIZE;
            break;
    }

    dma_cfg.hwid       = auart->thwid;
    dma_cfg.src_addr   = (u32)s;
    dma_cfg.dst_addr   = auart->base + SERIAL_TFIFO;
    dma_cfg.blk_len    = SERIAL_DMA_T_EMPTY;
    dma_cfg.mblk_len   = SERIAL_DMA_T_EMPTY;
    dma_cfg.frame_len  = n;
    dma_cfg.src_ds     = ds;
    dma_cfg.dst_ds     = ds;
    dma_cfg.trsf_type  = DMA_MEM_TO_DEV;
    dma_cfg.int_type   = DMA_FRM_INT;
    dma_cfg.req        = DMA_BLOCK_REQ;

    dcache_clean_by_range(DCLINE_ALIGN(s), DCLINE_GRANULARITY(s, n));

    AOS_INTERRUPT_DISABLE

    auart->tdmawork = 1;
    aic_dma_chn_cfg(auart->tdmachn, &dma_cfg);
    aic_dma_chn_start(auart->tdmachn);

    AOS_INTERRUPT_RESTORE
}

UART_TX_RET aic_uart_start_tx(struct uart_port *up)
{
    struct aic_uart *auart = (struct aic_uart *)up->data;
    u32 len;

    switch (auart->wsize) {
        case SERIAL_WSIZE4:
            up->tx_size = ALIGN_DOWN(up->tx_size, 4);
            break;
        case SERIAL_WSIZE2:
            up->tx_size = ALIGN_DOWN(up->tx_size, 2);
            break;
        case SERIAL_WSIZE1:
        default:
            break;
    }

    if (auart->tdmaeb && up->tx_size) {
        aic_uart_dma_tx(auart, up->tx_buffer, up->tx_size);
        return START_DMA;
    }

    while (up->tx_size) {
        if (aic_uart_tx_fifo_empty(auart)) {
            len = up->tx_size > SERIAL_FIFO_DEPTH ? SERIAL_FIFO_DEPTH : up->tx_size;
            aic_uart_write(up->data, up->tx_buffer, len);
            up->tx_buffer = (u8 *)up->tx_buffer + len;
            up->tx_size -= len;
        }
    }

    while(!aic_uart_tx_fifo_empty(auart));

    return NONE_IRQ;
}

void aic_uart_stop_tx(void *dev)
{
    struct aic_uart *auart = (struct aic_uart *)dev;

    AIC_WRITE32(auart->base + SERIAL_IDR, SERIAL_INT_TTRIG | SERIAL_TEMPTY);

    if (auart->tdmaeb)
        aic_dma_chn_free(auart->tdmachn);
}

void aic_uart_tx(struct uart_port *up, u32 status)
{
    u32 len = up->tx_size > SERIAL_FIFO_T_ROOM ? SERIAL_FIFO_T_ROOM : up->tx_size;
    struct aic_uart *auart = (struct aic_uart *)up->data;

    if (!up->tx_size) {
        AIC_WRITE32(auart->base + SERIAL_IDR, SERIAL_INT_TTRIG | SERIAL_TEMPTY);
        aos_set_event(up->event, UART_TX_EVENT_FLAG, AOS_OR);
    } else {
        aic_uart_write(up->data, up->tx_buffer, len);
        up->tx_buffer = (u8 *)up->tx_buffer + len;
        up->tx_size -= len;
        AIC_WRITE32(auart->base + SERIAL_IER, SERIAL_INT_TTRIG | SERIAL_TEMPTY);
    }
}

void aic_uart_check_timeout_irq(void *dev)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    u32 status = AIC_READ32(auart->base + SERIAL_CSR);

    if ((status & SERIAL_TIMEOUT) && !aic_uart_rx_fifo_empty(dev)) {
        aic_uart_timeout_set(dev, 0);
        aic_uart_timeout_set(dev, auart->rtor);
    }
}

void aic_uart_dma_rx(void *dev, char *s, u32 n)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    struct dma_chn_cfg dma_cfg = {0};
    u32 blk_len, ctrl;
    DMA_DATA_SIZE ds;

    AOS_INTERRUPT_DECLARE

    ctrl = AIC_READ32(auart->base + SERIAL_CTRL);
    ctrl |= SERIAL_DMA_RXEN;
    AIC_WRITE32(auart->base + SERIAL_CTRL, ctrl);

    switch (auart->wsize){
        case SERIAL_WSIZE4:
            ds = DMA_WORD_SIZE;
            break;
        case SERIAL_WSIZE2:
            ds = DMA_HWORD_SIZE;
            break;
        case SERIAL_WSIZE1:
        default:
            ds = DMA_BYTE_SIZE;
            break;
    }

    blk_len = 1 << (auart->wsize - 1);

    dma_cfg.hwid       = auart->rhwid;
    dma_cfg.src_addr   = auart->base + SERIAL_RFIFO;
    dma_cfg.dst_addr   = (u32)s;
    dma_cfg.blk_len    = blk_len;
    dma_cfg.mblk_len   = blk_len;
    dma_cfg.frame_len  = n;
    dma_cfg.src_ds     = ds;
    dma_cfg.dst_ds     = ds;
    dma_cfg.trsf_type  = DMA_DEV_TO_MEM;
    dma_cfg.int_type   = DMA_FRM_INT;
    dma_cfg.req        = DMA_BLOCK_REQ;

    dcache_invcc_by_range(DCLINE_ALIGN(s), DCLINE_GRANULARITY(s, n));

    AOS_INTERRUPT_DISABLE
    auart->rdmawork = 1;
    aic_dma_chn_cfg(auart->rdmachn, &dma_cfg);
    AIC_WRITE32(auart->base + SERIAL_IER, SERIAL_TIMEOUT);
    aic_uart_check_timeout_irq(dev);
    aic_dma_chn_start(auart->rdmachn);
    aic_dma_chn_resume(auart->rdmachn);
    AOS_INTERRUPT_RESTORE
}

UART_TX_RET aic_uart_start_rx(struct uart_port *up)
{
    struct aic_uart *auart = (struct aic_uart *)up->data;
    size_t len;

    switch (auart->wsize) {
        case SERIAL_WSIZE4:
            up->rx_size = ALIGN_DOWN(up->rx_size, 4);
            break;
        case SERIAL_WSIZE2:
            up->rx_size = ALIGN_DOWN(up->rx_size, 2);
            break;
        case SERIAL_WSIZE1:
        default:
            break;
    }

    if (auart->rdmaeb && up->rx_size) {
        aic_uart_dma_rx(auart, up->rx_buffer, up->rx_size);
        return START_DMA;
    }

    len = aic_uart_read(auart, up->rx_buffer, up->rx_size);
    up->rx_buffer = (u8 *)up->rx_buffer + len;
    up->rx_size -= len;

    return NONE_IRQ;
}

void aic_uart_stop_rx(void *dev)
{
    struct aic_uart *auart = (struct aic_uart *)dev;

    AIC_WRITE32(auart->base + SERIAL_IDR, SERIAL_RTRIG | SERIAL_TIMEOUT);

    if (auart->rdmaeb)
        aic_dma_chn_free(auart->rdmachn);
}

void aic_uart_rx(struct uart_port *up, u32 status)
{
    u32 event = UART_RX_EVENT_FLAG;

    if (status & SERIAL_TIMEOUT)
        event |= UART_TIMEOUT_EVENT_FLAG;
    if (status & SERIAL_RTRIG)
        event |= UART_RTRIG_EVENT_FLAG;

    aos_set_event(up->event, event, AOS_OR);
}

void aic_uart_irq_init(struct aic_uart *auart)
{
    if (!auart->rdmaeb) {
        AIC_WRITE32(auart->base + SERIAL_IER, SERIAL_RTRIG | SERIAL_TIMEOUT);
    }

#if 0
    AIC_WRITE32(auart->base + SERIAL_IER, SERIAL_INT_RBRK | SERIAL_PARE |
                                          SERIAL_FRAME);
#endif

    if (auart->inf == UART_INF2) {
        AIC_WRITE32(auart->base + SERIAL_IER, SERIAL_DMSI);
    }
}

void aic_uart_register_hook(UART_HOOK_FUNC func, void* param)
{
    UART_CB_T *uart_cb;

    uart_cb = aos_zalloc(sizeof(UART_CB_T));

    uart_cb->func = func;
    uart_cb->param = param;

    list_add(&uart_cb->node, &uart_cb_list);
}

void aic_uart_unregister_hook(UART_HOOK_FUNC func)
{
    struct list_head *pos, *next;
    UART_CB_T *uart_cb = NULL;

    list_for_each_safe(pos, next, &uart_cb_list) {
        uart_cb = list_entry(pos, UART_CB_T, node);
        if (uart_cb->func == func) {
            list_del(&uart_cb->node);
            aos_free(uart_cb);
        }
    }
}

ISR_RET_E aic_uart_irq(void *param)
{
    struct uart_port *up = (struct uart_port *)param;
    struct aic_uart *auart = (struct aic_uart *)up->data;
    struct list_head *pos, *next;
    UART_CB_T *uart_cb = NULL;
    boolean recfg = false;
    boolean slp = false;
    u32 status, len, ctrl;

    status = AIC_READ32(auart->base + SERIAL_CSR);
    if (status & SERIAL_ROVR)
        aicx_info(0x114c30, "x", "SERIAL overflow err = 0x%x\n", status);
    if (auart->rdmaeb && (status & SERIAL_TIMEOUT))
        AIC_WRITE32(auart->base + SERIAL_IDR, SERIAL_TIMEOUT);
    status = AIC_READ32(auart->base + SERIAL_CISR);

    if (status & SERIAL_INT_RBRK || status & SERIAL_PARE ||
        status & SERIAL_FRAME) {
        aic_uart_dma_uninit(up);
        aic_uart_hw_uninit(auart);
        aic_uart_hw_init(auart);
        aic_uart_startup(up, auart);
        aic_uart_dma_init(up);
        recfg = true;
    }

    if (status & SERIAL_TIMEOUT || status & SERIAL_RTRIG || recfg) {
        if (auart->rdmaeb) {
            AIC_WRITE32(auart->base + SERIAL_IDR, SERIAL_RTRIG);
            if (auart->rdmawork) {
                ctrl = AIC_READ32(auart->base + SERIAL_CTRL);
                ctrl &= ~SERIAL_DMA_RXEN;
                AIC_WRITE32(auart->base + SERIAL_CTRL, ctrl);
                /* suspend dma. */
                aic_dma_chn_suspend(auart->rdmachn);
                auart->rdmawork = 0;

                /* stop timeout irq. */

                /* get dma read len .*/
                len = aic_dma_chn_get_dst(auart->rdmachn) - (u32)up->rx_buffer;
                up->rx_buffer = (u8 *)up->rx_buffer + len;
                up->rx_size -= len;

                if (up->rx_size != up->rx_tsize) {
                    slp = true;
                }
                aic_uart_rx(up, status);
            } else {
                if (!aic_uart_rx_fifo_empty(auart) || up->wakeup) {
                    up->wakeup = false;
                    slp = true;
                    aic_uart_rx(up, status);
                }
            }
        } else {
            if (!aic_uart_rx_fifo_empty(auart) || up->wakeup) {
                up->wakeup = false;
                slp = true;
                aic_uart_rx(up, status);
            }
        }

        if (auart->eipu && (up->serial_status == SERIAL_WAKEUP) && slp) {
            u32 delaysleep = slp_get_delaysleep() * 1000;
            if (delaysleep) {
                aos_deactivate_timer(up->timer);
                aos_change_timer(up->timer, delaysleep, false);
                aos_activate_timer(up->timer);
            }
        }
    }

    if ((status & (SERIAL_INT_TTRIG | SERIAL_TEMPTY)) || recfg) {
        if (auart->tdmaeb) {
            auart->tdmawork = 0;
        }
        aic_uart_tx(up, status);
    }

    if (status & SERIAL_DMSI) {
        status = AIC_READ32(auart->base + SERIAL_MSR);
        status &= (SERIAL_DSR_STATUS | SERIAL_RI_STATUS | SERIAL_DCD_STATUS);
        if (status) {
            list_for_each_safe(pos, next, &uart_cb_list) {
                uart_cb = list_entry(pos, UART_CB_T, node);
                uart_cb->func(up->index, status, uart_cb->param);
            }
        }
    }

    return NONE_WORKER;
}

void aic_uart_dma_callback(void *param)
{
    struct dma_info *dma_info = (struct dma_info *)param;
    struct uart_port *up = (struct uart_port *)dma_info->up;
    struct aic_uart *auart = (struct aic_uart *)up->data;

    if ((dma_info->hwid == auart->thwid) && (auart->tdmawork)) {
        up->tx_buffer = (u8 *)up->tx_buffer + up->tx_size;
        up->tx_size = 0;
        auart->tdmawork = 0;
        aos_set_event(up->event, UART_TX_EVENT_FLAG, AOS_OR);
    } else if ((dma_info->hwid == auart->rhwid) && (auart->rdmawork)) {
        AIC_WRITE32(auart->base + SERIAL_IDR, SERIAL_TIMEOUT);
        up->rx_buffer = (u8 *)up->rx_buffer + up->rx_size;
        up->rx_size = 0;
        auart->rdmawork = 0;
        aos_set_event(up->event, UART_RX_EVENT_FLAG, AOS_OR);
    }
}

static void aic_uart_eipu_callback(u32 index, u32 value, u32 eipu_pol)
{
    u32 i, uart_max = sizeof(aic_uart) / sizeof(*aic_uart), delaysleep;
    struct uart_port *up;

    delaysleep = slp_get_delaysleep() * 1000;

    if (delaysleep == 0)
        return;

    for (i = 0; i < uart_max; i++) {
        if (aic_uart[i].eipu == index)
            break;
    }

    if (i == uart_max)
        return;

    up = uart_handle_get(i);
    up->serial_status = SERIAL_WAKEUP;

    aos_change_timer(up->timer, delaysleep, false);
    aos_activate_timer(up->timer);

    aos_send_to_queue(serial_thread_queue, &up->serial_status, AOS_NO_WAIT);
}

void aic_uart_startup(struct uart_port *up, struct aic_uart *auart)
{
    aic_uart_timeout_set(auart, auart->rtor);
    aic_uart_tx_trigger_level_set(auart, auart->ttr);
    aic_uart_rx_trigger_level_set(auart, auart->rtr);
    aic_uart_rx_fctrl_level_set(auart, auart->fcdr);
    aic_uart_irq_init(auart);

    register_irq(auart->irq, aic_uart_irq, (void *)up, NULL, NULL);
}

void aic_uart_dma_uninit(struct uart_port *up)
{
    struct aic_uart *auart = (struct aic_uart *)up->data;
    u32 len, ctrl;

    ctrl = AIC_READ32(auart->base + SERIAL_CTRL);

    if (auart->tdmaeb) {
        aic_dma_chn_free(auart->tdmachn);
        ctrl &= ~SERIAL_DMA_TXEN;
        AIC_WRITE32(auart->base + SERIAL_CTRL, ctrl);
    }

    if (auart->rdmaeb) {
        if (auart->rdmawork) {
            /* suspend dma. */
            aic_dma_chn_suspend(auart->rdmachn);
            auart->rdmawork = 0;

            /* get dma read len.*/
            len = aic_dma_chn_get_dst(auart->rdmachn) - (u32)up->rx_buffer;
            up->rx_buffer = (u8 *)up->rx_buffer + len;
            up->rx_size -= len;
        }

        ctrl &= ~SERIAL_DMA_RXEN;
        AIC_WRITE32(auart->base + SERIAL_CTRL, ctrl);
        aic_dma_chn_resume(auart->rdmachn);
        aic_dma_chn_free(auart->rdmachn);
    }
}

void aic_uart_dma_init(struct uart_port *up)
{
    struct aic_uart *auart = (struct aic_uart *)up->data;
    int chn;

    if (auart->dma_sel_reg)
        AIC_WRITE32(auart->dma_sel_reg, auart->dma_sel_val);

    if (auart->tdmaeb) {
        auart->tdmawork = 0;
        up->tdma_info.hwid = auart->thwid;
        up->tdma_info.up = up;
        chn = aic_dma_chn_request(aic_uart_dma_callback, &up->tdma_info);
        AIC_ASSERT(chn >= 0);
        auart->tdmachn = chn;
        AIC_WRITE32(auart->base + SERIAL_DMA_TEMPTY, SERIAL_DMA_T_EMPTY);
    }

    if (auart->rdmaeb) {
        if (auart->rdmawork) {
            auart->rdmawork = 0;
        }
        AIC_WRITE32(auart->base + SERIAL_IER, SERIAL_RTRIG);
        up->rdma_info.hwid = auart->rhwid;
        up->rdma_info.up = up;
        chn = aic_dma_chn_request(aic_uart_dma_callback, &up->rdma_info);
        AIC_ASSERT(chn >= 0);
        auart->rdmachn = chn;
        AIC_WRITE32(auart->base + SERIAL_DMA_RFULL, SERIAL_DMA_R_FULL);
    }
}

void aic_uart_hw_uninit(struct aic_uart *auart)
{
    if (AIC_READ32(auart->eb_reg)) {
        AIC_WRITE32(auart->base + SERIAL_IDR, 0xFFFFFFFF);
        AIC_WRITE32(auart->eb_reg, ~auart->eb_bit);
    }
}

void aic_uart_hw_init(struct aic_uart *auart)
{
    union uart_mode s_mode;
    u32 i, val;

    if (auart->source_clk != 26000000)
        clk_set_parent_comp(auart->parent_clk, auart->source_clk_index);

    AIC_WRITE32(auart->eb_reg, auart->eb_bit);

    AIC_WRITE32(auart->rst_reg, auart->rst_bit);
    i = 0;
    while (i < SERIAL_WAIT_TIME_CNT) {
        i++;
        val = AIC_READ32(auart->base + SERIAL_CTRL);
        if (!val)
            break;
        aic_mdelay(1);
    }
    AIC_ASSERT(!val);

    AIC_WRITE32(auart->rst_reg, 0);
    i = 0;
    while (i < SERIAL_WAIT_TIME_CNT) {
        i++;
        val = AIC_READ32(auart->base + SERIAL_CTRL);
        if (val)
            break;
        aic_mdelay(1);
    }
    AIC_ASSERT(val);

    aic_uart_baud_set((void *)auart, auart->baud);

    s_mode.mode = 0;
    s_mode.mode_bitmap.WSIZE = auart->wsize;
    s_mode.mode_bitmap.IRMODE = auart->IRmode;
    s_mode.mode_bitmap.UCLKEN = auart->uclken;
    s_mode.mode_bitmap.PAR = auart->parity;
    AIC_WRITE32(auart->base + SERIAL_MODE, s_mode.mode);

    if (auart->IRmode == SERIAL_IRDA) {
        aic_uart_rpwr_set(auart);
        aic_uart_tpwr_set(auart);
    }

    aic_uart_fctrl_set(auart, auart->fcm);

    AIC_WRITE32(auart->base + SERIAL_CTRL, SERIAL_STPBRK |
                SERIAL_RXEN | SERIAL_TXEN);
}

size_t aic_uart_write_dir(void *dev, const char *s, u32 n)
{
    struct aic_uart *auart = (struct aic_uart *)dev;
    u32 len, size = n, rsize;
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    while (size) {
        while (!aic_uart_tx_fifo_empty(auart));
        len = size < SERIAL_FIFO_DEPTH ?  size : SERIAL_FIFO_DEPTH;
        rsize = aic_uart_write(auart, s, len);
        if (!rsize)
            break;
        s = s + rsize;
        size -= rsize;
    }
    AOS_INTERRUPT_RESTORE

    return (n - size);
}

void aic_uart_stop(struct uart_port *up)
{
    struct aic_uart *auart = (struct aic_uart *)up->data;

    AOS_INTERRUPT_DECLARE;
    AOS_INTERRUPT_DISABLE;
    /* clean data and disable tx & rx. */
    aic_uart_stop_tx(auart);
    aic_uart_stop_rx(auart);
    AIC_WRITE32(auart->base + SERIAL_CTRL, SERIAL_RXRES | SERIAL_TXRES);
    AOS_INTERRUPT_RESTORE;

    aos_set_event(up->event, UART_TX_EVENT_FLAG, AOS_OR);
    aos_set_event(up->event, UART_RX_EVENT_FLAG, AOS_OR);

    AIC_WRITE32(auart->eb_reg, 0);

    unregister_irq(auart->irq);

    if (auart->eipu)
        EIPU_UnRegCallback(auart->eipu, EIPU_LATCH, NOSEC_MODE);
}

void aic_uart_suspend(struct uart_port *up)
{
    struct aic_uart *auart = (struct aic_uart *)up->data;
    u32 clk, len, csr, slen;

    while(!aic_uart_tx_fifo_empty(auart));

    slen = up->rx_size;

    aic_uart_dma_uninit(up);

    while (up->rx_size) {
        len = aic_uart_read(auart, up->rx_buffer, up->rx_size);
        up->rx_buffer = (u8 *)up->rx_buffer + len;
        up->rx_size -= len;

        csr = AIC_READ32(auart->base + SERIAL_CSR);
        if ((csr & SERIAL_CSR_RACTIVE) ||  (csr & SERIAL_CSR_TACTIVE))
            continue;
        else
            break;
    }

    if (auart->base == REG_BASE_AON_UART) {
        if (auart->source_clk != SERIAL_SLP_SOURCE_CLK) {
            /* set clock 32K. */
            aon_clk_index = clk_get_parent_comp(CLK_TOP_PWM0);
            clk_set_parent_comp(CLK_TOP_PWM0, 2);
            clk = auart->source_clk;
            auart->source_clk = SERIAL_SLP_SOURCE_CLK;
            aic_uart_baud_set(auart, auart->baud);
            auart->source_clk = clk;
        }
    } else {
        AIC_READ32(auart->base + SERIAL_CISR);
        aic_uart_hw_uninit(auart);
    }

    if (slen != up->rx_size)
        up->wakeup = true;

    if (auart->eipu)
        EIPU_RegCallback_Ex(auart->eipu, EIPU_LATCH, aic_uart_eipu_callback,
                            LOW_LEVEL, 0, NOSEC_MODE, SLEEP_WK_MODE);
}

void aic_uart_resume(struct uart_port *up)
{
    struct aic_uart *auart = (struct aic_uart *)up->data;

    if (auart->base == REG_BASE_AON_UART) {
        clk_set_parent_comp(CLK_TOP_PWM0, aon_clk_index);
        aic_uart_baud_set(auart, auart->baud);
    } else {
        aic_uart_hw_init(auart);
        aic_uart_startup(up, auart);
    }

    aic_uart_dma_init(up);
    if (up->rx_size)
        aic_uart_start_rx(up);
}

void aic_uart_open(struct uart_port *up)
{
    struct aic_uart *auart = (struct aic_uart *)up->data;

    if (!auart->opencnt) {
        aic_uart_iomux_sel(auart, auart->dst_inf);
        aic_uart_hw_init(auart);
        aic_uart_dma_init(up);
        aic_uart_startup(up, auart);
    }
    ++auart->opencnt;
}

void aic_uart_close(struct uart_port *up)
{
    struct aic_uart *auart = (struct aic_uart *)up->data;

    --auart->opencnt;
    if (!auart->opencnt) {
        aic_uart_stop(up);
    }
}

/**
 * serial_timeout - receive new data timeout.
 *
 * Parameter:
 *  @input: input param..
 *
 * Return:
 *
 */
static void aic_serial_recv_data_timeout(void *input)
{
    struct uart_port *up = (struct uart_port *)input;
    struct aic_uart *auart = (struct aic_uart *)up->data;

    AOS_INTERRUPT_DECLARE;

    AOS_INTERRUPT_DISABLE;
    if (auart->eipu && (up->serial_status == SERIAL_WAKEUP)) {
        up->serial_status = SERIAL_SLEEP;
        AOS_INTERRUPT_RESTORE;
        aos_send_to_queue(serial_thread_queue, &up->serial_status, AOS_NO_WAIT);
    } else {
        AOS_INTERRUPT_RESTORE;
    }
}

/**
 * aic_serial_entry - serial task.
 *
 * @param
 *
 * Return:
 *
 */
static void aic_serial_entry(void *param)
{
    u32 ret, serial_status;

    while(1) {
        ret = aos_recv_from_queue(serial_thread_queue, &serial_status,
                                  AOS_WAIT_FOREVER);
        if (ret)
            continue;

        switch (serial_status) {
            case SERIAL_SLEEP:
                aic_put_wakelock();
                break;
            case SERIAL_WAKEUP:
                aic_get_wakelock();
                break;
        }
    }
}

int aic_uart_init(void)
{
    u32 uart_max = sizeof(aic_uart) / sizeof(*aic_uart);
    struct uart_port *up;
    u32 i, delaysleep;

    up = aos_zalloc(sizeof(struct uart_port) * uart_max);

    delaysleep = slp_get_delaysleep() * 1000;
    if (delaysleep == 0)
        delaysleep = SERIAL_RECV_DATA_TIMEOUT;

    for (i = 0; i < uart_max; i++) {
        aic_uart[i].opencnt = SERIAL_UNUSED;
        aic_uart[i].tdmawork = 0;
        aic_uart[i].rdmawork = 0;
        up[i].name = aic_uart[i].name;
        up[i].inf = aic_uart[i].inf;
        up[i].data = (void *)&aic_uart[i];
#ifdef CONFIG_SUBSYS_AP
        aic_uart_hw_uninit(&aic_uart[i]);
#endif
        up[i].index = i;

        if (aic_uart[i].eipu) {
            up[i].timer = aos_create_timer(up[i].name,
                                aic_serial_recv_data_timeout,
                                &up[i], delaysleep,
                                AOS_NO_ACTIVATE, AOS_NO_PERIOD);
            AIC_ASSERT(up[i].timer != AOS_NULL);
        }
    }

    uart_add_port(up, uart_max);

    list_head_init(&uart_cb_list);

    serial_thread_id = aos_create_thread("uart",
                                            aic_serial_entry, AOS_NULL,
                                            SERIAL_TASK_STACK_SIZE, 0,
                                            AIC_NULL, SERIAL_TASK_PRIORITY,
                                            AOS_AUTO_START);
    AIC_ASSERT(AOS_INVALID_THREAD_ID != serial_thread_id);

    serial_thread_queue = aos_create_queue("uart", sizeof(u32),
                                            SERIAL_TASK_QUEUE_CNT);
    AIC_ASSERT(AOS_NULL != serial_thread_queue);

    return AIC_SUCCESS;
}

