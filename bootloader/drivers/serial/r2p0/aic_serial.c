/*****************************************************************************/
/*                                                                           */
/*  Copyright2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
/*                                                                           */
/*****************************************************************************/

#include <cru_top.h>
#include <timer.h>
#include "aic_io.h"
#include "console/console.h"
#include "initcall.h"
#include "platform/platform.h"
#include "serial/serial_cfg.h"

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
#define SERIAL_RFIFO_NUM    0x5C
#define SERIAL_TFIFO_NUM    0x60

/* ctrl */
#define SERIAL_INT_CLR_MODE BIT_12
#define SERIAL_TIMEOUT_MODE BIT_11
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

#define SERIAL_USBD         1
#define SERIAL_UNUSBD       0

#define BAUD_SAMPLE(s,c) ((s) / (c))
#define BDIV(a,b) (((((a) * 10) / (b) + 5) / 10) - 1)

#define SERIAL_xTRIG(v)     ((v) & 0x3f)

#define SERIAL_TIMEOUT_SYCLE    0x10

union uart_mode {
    u32 mode;
    struct{
        u32 CLKS : 1;
        u32 CHRL : 2;
        u32 PAR : 3;
        u32 NBSTOP : 2;
        u32 CHMODE : 2;
        u32 UCLKEN : 1;
        u32 IRMODE : 1;
        u32 WSIZE : 2;
        u32 RSV : 18;
    } mode_bitmap;
};

union lpuart_rx_cfg0 {
    u32 rx_cfg0;
    struct{
        u32 RTOR : 8;
        u32 BDIVR : 8;
        u32 BRGR : 16;
    } rx_cfg0_bitmap;
};

union lpuart_rx_cfg1 {
    u32 rx_cfg1;
    struct{
        u32 RXEN : 1;
        u32 DEBREN : 1;
        u32 CHMODE : 2;
        u32 CLKS : 1;
        u32 PAR : 3;
        u32 CHRL : 2;
        u32 RSV : 22;
    } rx_cfg1_bitmap;
};

#ifdef LOG_BUF_SIZE
uint8_t log_buf[LOG_BUF_SIZE];
uint32_t log_index = 0;
#endif

static void aic_uart_timeout_set(struct aic_uart *aserial, u32 val)
{
    AIC_WRITE32(aserial->base + SERIAL_RTOR, val);

    if (aserial->base == REG_BASE_TOP_TOP_UART_TX) {
        union lpuart_rx_cfg0 rx_cfg0;

        rx_cfg0.rx_cfg0 = AIC_READ32(REG_SYS_ND_GLB_LPUART_RX_CFG0);
        rx_cfg0.rx_cfg0_bitmap.RTOR = val;
        AIC_WRITE32(REG_SYS_ND_GLB_LPUART_RX_CFG0, rx_cfg0.rx_cfg0);
    }
}

static void aic_uart_console_putc(struct console_dev *dev, char ch)
{
    struct aic_uart *auart = (struct aic_uart *)dev->data;

#ifdef LOG_BUF_SIZE
    log_buf[log_index++] = ch;
    log_index = (log_index >= LOG_BUF_SIZE) ? 0 : log_index;
#endif

    while (!(AIC_READ32(auart->base + SERIAL_CSR) & SERIAL_TEMPTY));
    AIC_WRITE32(auart->base + SERIAL_TFIFO, ch);
}

static int aic_is_serial_tx_fifo_full(struct aic_uart *aserial)
{
    u32 status;

    status = AIC_READ32(aserial->base + SERIAL_CSR);

    if (status & SERIAL_TFUL)
        return true;

    return false;
}

static void aic_serial_putchar(struct aic_uart *aserial, u8 ch)
{
    AIC_WRITE32(aserial->base + SERIAL_TFIFO, ch);
}

void aic_uart_write(void *data, u8 *buf, u32 len)
{
    struct aic_uart *aserial = (struct aic_uart *)data;
    u32 i;

    for (i = 0; i < len; i++) {
        while(aic_is_serial_tx_fifo_full(aserial))
            ;
        aic_serial_putchar(aserial, buf[i]);
    }
}

static u8 aic_serial_getchar(struct aic_uart *aserial)
{
    return AIC_READ32(aserial->base + SERIAL_RFIFO);
}

u8 aic_uart_getchar(void *data)
{
    return aic_serial_getchar((struct aic_uart *)data);
}

static u32 aic_serial_1byte_rx(struct aic_uart *aserial, u8 *buf)
{
    u32 i = 0;

    while(!(AIC_READ32(aserial->base + SERIAL_CSR) & SERIAL_REMPTY)) {
        buf[i] = AIC_READ32(aserial->base + SERIAL_RFIFO);
        i++;
    }

    return i;
}

void aic_uart_clr_status(void *data)
{
    struct aic_uart *aserial = (struct aic_uart *)data;

    AIC_READ32(aserial->base + SERIAL_CISR);
}

u32 aic_uart_read(void *data, u8 *buf)
{
    return aic_serial_1byte_rx((struct aic_uart *)data, buf);
}

u32 aic_is_uart_rx_fifo_triger(void *dev)
{
    struct aic_uart *auart = (struct aic_uart *)dev;

    return AIC_READ32(auart->base + SERIAL_CSR) & SERIAL_RTRIG;
}

void aic_uart_rx_trigger_level_register(void *dev, u32 val)
{
    struct aic_uart *auart = (struct aic_uart *)dev;

    AIC_WRITE32(auart->base + SERIAL_RTR, SERIAL_xTRIG(val));
}

static int aic_is_serial_rx_fifo_empty(struct aic_uart *aserial)
{
    u32 status;

    status = AIC_READ32(aserial->base + SERIAL_CSR);

    if (status & SERIAL_REMPTY)
        return true;

    return false;
}

static int aic_is_serial_rx_fifo_timeout(struct aic_uart *aserial)
{
    u32 status;

    status = AIC_READ32(aserial->base + SERIAL_CSR);

    if (status & SERIAL_TIMEOUT)
        return true;

    return false;
}

static int aic_is_serial_tx_fifo_empty(struct aic_uart *aserial)
{
    u32 status;

    status = AIC_READ32(aserial->base + SERIAL_CSR);

    if (status & SERIAL_TEMPTY)
        return true;

    return false;
}

int aic_is_uart_recv_fifo_empty(void *data)
{
    return aic_is_serial_rx_fifo_empty((struct aic_uart *)data);
}

int aic_is_uart_recv_fifo_timeout(void *data)
{
    return aic_is_serial_rx_fifo_timeout((struct aic_uart *)data);
}

int aic_is_uart_send_fifo_empty(void *data)
{
    return aic_is_serial_tx_fifo_empty((struct aic_uart *)data);
}

void *aic_uart_port_open(int port)
{
    u32 uart_max = sizeof(aic_uart) / sizeof(*aic_uart);

    if (port >= uart_max)
        return NULL;

    return &aic_uart[port];
}

void aic_uart_baud_set(void *dev, u32 baud)
{
    u32 bdiv, minbdiv, cd, baud_sample, i, tmp_bdiv, ctrl;
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

    if (auart->base == REG_BASE_TOP_TOP_UART_TX) {
        union lpuart_rx_cfg0 rx_cfg0;
        union lpuart_rx_cfg1 rx_cfg1;

        rx_cfg0.rx_cfg0 = AIC_READ32(REG_SYS_ND_GLB_LPUART_RX_CFG0);
        rx_cfg0.rx_cfg0_bitmap.BRGR = auart->cd;
        rx_cfg0.rx_cfg0_bitmap.BDIVR = bdiv;
        AIC_WRITE32(REG_SYS_ND_GLB_LPUART_RX_CFG0, rx_cfg0.rx_cfg0);

        rx_cfg1.rx_cfg1 = AIC_READ32(REG_SYS_ND_GLB_LPUART_RX_CFG1);
        rx_cfg1.rx_cfg1_bitmap.DEBREN = 0;
        AIC_WRITE32(REG_SYS_ND_GLB_LPUART_RX_CFG1, rx_cfg1.rx_cfg1);
    }

    ctrl = AIC_READ32(auart->base + SERIAL_CTRL);
    ctrl &= ~(SERIAL_RXEN | SERIAL_TXEN);
    ctrl |= (SERIAL_RXDIS | SERIAL_TXDIS);
    AIC_WRITE32(auart->base + SERIAL_CTRL, ctrl);
    for (i = 0; i < 100; i++) {
        AIC_WRITE32(auart->base + SERIAL_BRGR, auart->cd);
        AIC_WRITE32(auart->base + SERIAL_BDIVR, bdiv);

        cd = AIC_READ32(auart->base + SERIAL_BRGR);
        tmp_bdiv = AIC_READ32(auart->base + SERIAL_BDIVR);
        if (auart->cd == cd && bdiv == tmp_bdiv)
            break;
    }
    ctrl &= ~(SERIAL_RXDIS | SERIAL_TXDIS);
    ctrl |= (SERIAL_RXEN | SERIAL_TXEN);
    AIC_WRITE32(auart->base + SERIAL_CTRL, ctrl);
    ASSERT(i < 100);

    auart->baud = baud;
}

void aic_uart_hw_init(struct aic_uart *auart)
{
    union lpuart_rx_cfg1 rx_cfg1;
    union uart_mode s_mode;
    boolean rxoff = true;

#ifdef CONFIG_DL
    return;
#endif

    AIC_WRITE32(uart_clk_comp_array[auart->parent_clk].reg,
                uart_clk_comp_array[auart->parent_clk].parents[auart->source_clk_index].index);
    if (auart->nd_parent_clk) {
        AIC_WRITE32(uart_clk_comp_array[auart->nd_parent_clk].reg,
                    uart_clk_comp_array[auart->nd_parent_clk].parents[auart->nd_source_clk_index].index);
    }

    AIC_WRITE32(auart->eb_reg, auart->eb_bit);

    if (auart->base == REG_BASE_TOP_TOP_UART_TX) {
        AIC_WRITE32(REG_SYS_ND_GLB_RF_INT_EN_LPUART, 0x0);
        AIC_WRITE32(REG_SYS_ND_GLB_RF_INT_CLR_LPUART, BIT_SYS_ND_GLB_RF_INT_CLR_LPUART);

        AIC_WRITE32((void *)(REG_SYS_ND_GLB_ND_UART_RX_EB), BIT_SYS_ND_GLB_ND_UART_RX_EB);
        AIC_WRITE32(REG_SYS_ND_GLB_LPUART_RX_TIMEOUT, 0x0);

        rx_cfg1.rx_cfg1 = AIC_READ32(REG_SYS_ND_GLB_LPUART_RX_CFG1);

        rx_cfg1.rx_cfg1_bitmap.RXEN = 1;
        rx_cfg1.rx_cfg1_bitmap.CLKS = 0;
        rx_cfg1.rx_cfg1_bitmap.PAR = auart->parity;
        AIC_WRITE32(REG_SYS_ND_GLB_LPUART_RX_CFG1, rx_cfg1.rx_cfg1);
    }

    aic_uart_baud_set((void *)auart, auart->baud);

    s_mode.mode = 0;
    s_mode.mode_bitmap.WSIZE = auart->wsize;
    s_mode.mode_bitmap.IRMODE = auart->IRmode;
    s_mode.mode_bitmap.UCLKEN = auart->uclken;
    s_mode.mode_bitmap.PAR = auart->parity;
    AIC_WRITE32(auart->base + SERIAL_MODE, s_mode.mode);

    AIC_WRITE32(auart->base + SERIAL_CTRL, SERIAL_INT_CLR_MODE | SERIAL_TIMEOUT_MODE | SERIAL_RXEN | SERIAL_TXEN);

    if (auart->base == REG_BASE_TOP_TOP_UART_TX) {
        do {
            AIC_WRITE32(REG_SYS_ND_GLB_LPUART_RX_EN_CFG_CLR, BIT_SYS_ND_GLB_RF_LPUART_NON_ND_EB_OFF_REQ);
            rxoff = AIC_READ32(REG_SYS_ND_GLB_LPUART_RX_EN_CFG) & BIT_SYS_ND_GLB_RF_LPUART_NON_ND_EB_OFF_ACK;
        } while(rxoff);
    }

    aic_uart_timeout_set(auart, SERIAL_TIMEOUT_SYCLE);
}

static int aic_uart_console_setup(struct console_dev *dev)
{
    struct aic_uart *au = (struct aic_uart*)dev->data;

    aic_uart_hw_init(au);
    aic_uart_baud_set(au, au->baud);

    return 0;
}

static struct console_dev aic_console = {
    .name = "cons_serial",
    .putc = aic_uart_console_putc,
    .data = &aic_uart[SERIAL_CONSOLE_INDEX],
    .setup =aic_uart_console_setup,
};

static int aic_uart_console_init(void)
{
    console_register(&aic_console);

    return 0;
}

core_init(aic_uart_console_init);

static int aic_uart_init(void)
{
    u32 uart_max = sizeof(aic_uart) / sizeof(*aic_uart);
    int i;

    for (i = 0;  i < uart_max; i++)
        aic_uart_hw_init(&aic_uart[i]);

    return 0;
}

core_init(aic_uart_init);
