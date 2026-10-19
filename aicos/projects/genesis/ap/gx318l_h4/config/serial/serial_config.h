/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __SERIAl_CFG_H__
#define __SERIAl_CFG_H__

#include <reg_base.h>
#include <mcu_glb.h>
#include <aon_glb.h>
#include <top_glb.h>
#include <dma.h>
#include <eipu.h>
#include <aic_clk_map.h>

/* mode */
#define SERIAL_WSIZE1OR2            0
#define SERIAL_WSIZE1               1
#define SERIAL_WSIZE2               2
#define SERIAL_WSIZE4               3

#define SERIAL_UART                 0
#define SERIAL_IRDA                 1

#define SERIAL_SRC_PCLK             0
#define SERIAL_SRC_UCLK             1

#define SERIAL_FLOW_CTRL_DISABLE    0
#define SERIAL_FLOW_CTRL_ENABLE     1
#define SERIAL_FIFO_R_FCTL          56

#define SERIAL_IOMUX_SEL            (REG_BASE_AON_IO_MUX + 0x10)
#define SERIAL_INF0                 0
#define SERIAL_INF1                 1
#define SERIAL_INF2                 2
#define SERIAL_INF3                 3
#define SERIAL_INF4                 4
#define SERIAL_INF5                 5

#define SERIAL_DMA_ENABLE           1
#define SERIAL_DMA_DISABLE          0
#define SERIAL_DMA_R_FULL           1
#define SERIAL_DMA_T_EMPTY          8

#define SERIAL_TIMEOUT_CYCLE        0x10//0xFF
#define SERIAL_FIFO_T_LEVEL         8
#define SERIAL_FIFO_T_ROOM          56
#define SERIAL_FIFO_R_LEVEL         32

enum SERIAL_DMA_SEL {
    DMA_SEL_AP = 0,
    DMA_SEL_AUD,
    DMA_SEL_CP,
};

enum PAR{
    EVEN_PARITY,
    ODD_PARITY,
    FORCE_0_PARITY,
    FORCE_1_PARITY,
    NO_PARITY,
};

enum SERIAL_CLK{
    SERIAL_100M_CLK = 100000000,
    SERIAL_26M_CLK = 26000000,
    SERIAL_32K_CLK = 32000
};

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

struct aic_uart {
    char *name;
    u32 inf;
    u32 dst_inf;
    u32 opencnt;
    u32 base;
    u32 source_clk;
    u32 irq;
    u32 tdmaeb;
    u32 tdmachn;
    u32 tdmawork;
    u32 thwid;
    u32 ttr;
    u32 rdmaeb;
    u32 rdmachn;
    u32 rdmawork;
    u32 rhwid;
    u32 rtor;
    u32 rtr;
    u32 wsize;
    u32 IRmode;
    u32 uclken;
    u32 parity;
    u32 baud;
    u32 cd;
    u32 fcm;
    u32 fcdr;
    u32 eb_reg;
    u32 eb_bit;
    u32 rst_reg;
    u32 rst_bit;
    u32 dma_sel_reg;
    u32 dma_sel_val;
    u32 eipu;
    u32 parent_clk;
    u32 source_clk_index;
};

#define SERIAL_SLP_SOURCE_CLK   SERIAL_32K_CLK

#define SERIAL_CONSOLE_INDEX    0

struct aic_uart aic_uart[6] = {
    {
        .name = "uart0",
        .inf = SERIAL_INF3,
        .dst_inf = SERIAL_INF3,
        .base = REG_BASE_AP_UARTC0,
        .baud = 8000000,
        .source_clk = SERIAL_100M_CLK,
        .irq = IRQ_UART0,
        .wsize = SERIAL_WSIZE1,
        .tdmaeb = SERIAL_DMA_ENABLE,
        .thwid = DMA_REQ_MCU_UART0_TX,
        .ttr = SERIAL_FIFO_T_LEVEL,
        .rdmaeb = SERIAL_DMA_ENABLE,
        .rhwid = DMA_REQ_MCU_UART0_RX,
        .rtor = SERIAL_TIMEOUT_CYCLE,
        .rtr = SERIAL_FIFO_R_LEVEL,
        .IRmode = SERIAL_UART,
        .uclken = SERIAL_SRC_UCLK,
        .parity = NO_PARITY,
        .fcm = SERIAL_FLOW_CTRL_DISABLE,
        .fcdr = SERIAL_FIFO_R_FCTL,
        .eb_reg = REG_MCU_GLB_UART_0_EB,
        .eb_bit = BIT_MCU_GLB_UART0_EB,
        .rst_reg = REG_MCU_GLB_SW_RST_GLB_CLK_MCU_UART0,
        .rst_bit = BIT_MCU_GLB_SW_RST_GLB_CLK_MCU_UART0,
        .parent_clk = CLK_MCU_UART0,
        .source_clk_index = 0,
    }, {
        .name = "uart1",
        .inf = SERIAL_INF4,
        .dst_inf = SERIAL_INF4,
        .base = REG_BASE_AP_UARTC1,
        .baud = 2000000,
        .source_clk = SERIAL_26M_CLK,
        .irq = IRQ_UART1,
        .wsize = SERIAL_WSIZE1,
        .tdmaeb = SERIAL_DMA_ENABLE,
        .thwid = DMA_REQ_MCU_UART1_TX,
        .ttr = SERIAL_FIFO_T_LEVEL,
        .rdmaeb = SERIAL_DMA_ENABLE,
        .rhwid = DMA_REQ_MCU_UART1_RX,
        .rtor = SERIAL_TIMEOUT_CYCLE,
        .rtr = SERIAL_FIFO_R_LEVEL,
        .IRmode = SERIAL_UART,
        .uclken = SERIAL_SRC_UCLK,
        .parity = NO_PARITY,
        .fcm = SERIAL_FLOW_CTRL_DISABLE,
        .fcdr = SERIAL_FIFO_R_FCTL,
        .eb_reg = REG_MCU_GLB_UART_1_EB,
        .eb_bit = BIT_MCU_GLB_UART1_EB,
        .rst_reg = REG_MCU_GLB_SW_RST_GLB_CLK_MCU_UART1,
        .rst_bit = BIT_MCU_GLB_SW_RST_GLB_CLK_MCU_UART1,
        .parent_clk = CLK_MCU_UART1,
        .source_clk_index = 1,
   }, {
        .name = "uart2",
        .inf = SERIAL_INF5,
        .dst_inf = SERIAL_INF5,
        .base = REG_BASE_AP_UARTC2,
        .baud = 2000000,
        .source_clk = SERIAL_26M_CLK,
        .irq = IRQ_UART2,
        .wsize = SERIAL_WSIZE1,
        .tdmaeb = SERIAL_DMA_ENABLE,
        .thwid = DMA_REQ_MCU_UART2_TX,
        .ttr = SERIAL_FIFO_T_LEVEL,
        .rdmaeb = SERIAL_DMA_ENABLE,
        .rhwid = DMA_REQ_MCU_UART2_RX,
        .rtor = SERIAL_TIMEOUT_CYCLE,
        .rtr = SERIAL_FIFO_R_LEVEL,
        .IRmode = SERIAL_UART,
        .uclken = SERIAL_SRC_UCLK,
        .parity = NO_PARITY,
        .fcm = SERIAL_FLOW_CTRL_DISABLE,
        .fcdr = SERIAL_FIFO_R_FCTL,
        .eb_reg = REG_MCU_GLB_UART_2_EB,
        .eb_bit = BIT_MCU_GLB_UART2_EB,
        .rst_reg = REG_MCU_GLB_SW_RST_GLB_CLK_MCU_UART2,
        .rst_bit = BIT_MCU_GLB_SW_RST_GLB_CLK_MCU_UART2,
        .eipu = EXTINT_UART5,
        .parent_clk = CLK_MCU_UART2,
        .source_clk_index = 1,
   }, {
        .name = "uart3",
        .inf = SERIAL_INF0,
        .dst_inf = SERIAL_INF0,
        .base = REG_BASE_AON_UART,
        .baud = 8192,
        .source_clk = SERIAL_100M_CLK,
        .irq = IRQ_AON_UART,
        .wsize = SERIAL_WSIZE1,
        .tdmaeb = SERIAL_DMA_ENABLE,
        .thwid = DMA_REQ_AON_UART_TX,
        .ttr = SERIAL_FIFO_T_LEVEL,
        .rdmaeb = SERIAL_DMA_ENABLE,
        .rhwid = DMA_REQ_AON_UART_RX,
        .rtor = SERIAL_TIMEOUT_CYCLE,
        .rtr = SERIAL_FIFO_R_LEVEL,
        .IRmode = SERIAL_UART,
        .uclken = SERIAL_SRC_UCLK,
        .parity = NO_PARITY,
        .fcm = SERIAL_FLOW_CTRL_DISABLE,
        .fcdr = SERIAL_FIFO_R_FCTL,
        .eb_reg = REG_AON_GLB_UART_EB,
        .eb_bit = BIT_AON_GLB_UART_EB,
        .rst_reg = REG_TOP_GLB_SW_RST_AON_UART,
        .rst_bit = BIT_TOP_GLB_SW_RST_GLB_CLK_AON_UART,
        .parent_clk = CLK_TOP_PWM0,
        .source_clk_index = 0,
   }, {
        .name = "uart4",
        .inf = SERIAL_INF1,
        .dst_inf = SERIAL_INF1,
        .base = REG_BASE_TOP_UART0,
        .baud = 8000000,
        .source_clk = SERIAL_100M_CLK,
        .irq = IRQ_TOP_UART0,
        .wsize = SERIAL_WSIZE1,
        .tdmaeb = SERIAL_DMA_ENABLE,
        .thwid = DMA_REQ_TOP_UART0_TX,
        .ttr = SERIAL_FIFO_T_LEVEL,
        .rdmaeb = SERIAL_DMA_ENABLE,
        .rhwid = DMA_REQ_TOP_UART0_RX,
        .rtor = SERIAL_TIMEOUT_CYCLE,
        .rtr = SERIAL_FIFO_R_LEVEL,
        .IRmode = SERIAL_UART,
        .uclken = SERIAL_SRC_UCLK,
        .parity = NO_PARITY,
        .fcm = SERIAL_FLOW_CTRL_DISABLE,
        .fcdr = SERIAL_FIFO_R_FCTL,
        .eb_reg = REG_TOP_GLB_UART0_EB,
        .eb_bit = BIT_TOP_GLB_UART0_EB,
        .rst_reg = REG_TOP_GLB_SW_RST_TOP_UART0,
        .rst_bit = BIT_TOP_GLB_SW_RST_GLB_CLK_TOP_UART0,
        .dma_sel_reg = REG_TOP_GLB_DMA_SEL_UART0,
        .dma_sel_val = DMA_SEL_AP,
        .parent_clk = CLK_TOP_PWM0,
        .source_clk_index = 0,
   }, {
        .name = "uart5",
        .inf = SERIAL_INF2,
        .dst_inf = SERIAL_INF2,
        .base = REG_BASE_TOP_UART1,
        .baud = 115200,
        .source_clk = SERIAL_100M_CLK,
        .irq = IRQ_TOP_UART1,
        .wsize = SERIAL_WSIZE1,
        .tdmaeb = SERIAL_DMA_ENABLE,
        .thwid = DMA_REQ_TOP_UART1_TX,
        .ttr = SERIAL_FIFO_T_LEVEL,
        .rdmaeb = SERIAL_DMA_ENABLE,
        .rhwid = DMA_REQ_TOP_UART1_RX,
        .rtor = SERIAL_TIMEOUT_CYCLE,
        .rtr = SERIAL_FIFO_R_LEVEL,
        .IRmode = SERIAL_UART,
        .uclken = SERIAL_SRC_UCLK,
        .parity = NO_PARITY,
        .fcm = SERIAL_FLOW_CTRL_DISABLE,
        .fcdr = SERIAL_FIFO_R_FCTL,
        .eb_reg = REG_TOP_GLB_UART1_EB,
        .eb_bit = BIT_TOP_GLB_UART1_EB,
        .rst_reg = REG_TOP_GLB_SW_RST_TOP_UART1,
        .rst_bit = BIT_TOP_GLB_SW_RST_GLB_CLK_TOP_UART1,
        .dma_sel_reg = REG_TOP_GLB_DMA_SEL_UART1,
        .dma_sel_val = DMA_SEL_AP,
        .eipu = EXTINT_UART2,
        .parent_clk = CLK_TOP_PWM0,
        .source_clk_index = 0,
   }
};

#endif /* __SERIAl_CFG_H__ */
