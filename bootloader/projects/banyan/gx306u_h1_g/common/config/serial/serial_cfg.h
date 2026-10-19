/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#ifndef __SERIAL_CFG_H__
#define __SERIAL_CFG_H__

#include <reg_base.h>
#include <sys_nd_glb.h>
#include <ckm_en_glb_top.h>
#include <ckm_en_glb_mcu.h>
#include <cru_top.h>
#include <cru_mcu.h>
#include <aic_type.h>

/* mode */
#define SERIAL_WSIZE1OR2    0
#define SERIAL_WSIZE1       1
#define SERIAL_WSIZE2       2
#define SERIAL_WSIZE3       3

#define SERIAL_UART         0
#define SERIAL_IRDA         1

#define SERIAL_SRC_PCLK     0
#define SERIAL_SRC_UCLK     1

#define SERIAL_INF0         0
#define SERIAL_INF1         1
#define SERIAL_INF2         2
#define SERIAL_INF3         3

enum PAR{
    EVEN_PARITY,
    ODD_PARITY,
    FORCE_0_PARITY,
    FORCE_1_PARITY,
    NO_PARITY,
};

enum SERIAL_CLK{
    SERIAL_96M_CLK  = 96000000,
    SERIAL_26M_CLK  = 26000000,
    SERIAL_460K_CLK = 460800,
    SERIAL_32K_CLK  = 32768
};

enum SERIAL_CLK_ID {
    CLK_TOP_LPUART_PD_CFG = 0,
    CLK_ND_UART_RX,
    CLK_TOP_UART_CFG,
    CLK_MCU_UART0_CFG,
    CLK_MCU_UART1_CFG
};

struct uart_clk_parent {
    u8 index;
    u32 rate;
};

const struct uart_clk_parent clk_top_lpuart_pd_parents[] = {
    {
        .index = 0,
        .rate = 26000000,
    },
    {
        .index = 1,
        .rate = 460800,
    },
    {
        .index = 2,
        .rate = 32768,
    },
};

const struct uart_clk_parent clk_top_uart_parents[] = {
    {
        .index = 0,
        .rate = 96000000,
    },
    {
        .index = 1,
        .rate = 26000000,
    },
    {
        .index = 2,
        .rate = 460800,
    },
    {
        .index = 3,
        .rate = 32768,
    },
};

const struct uart_clk_parent clk_mcu_uart_parents[] = {
    {
        .index = 0,
        .rate = 96000000,
    },
    {
        .index = 1,
        .rate = 26000000,
    },
    {
        .index = 2,
        .rate = 460800,
    },
};

struct uart_clk_comp {
    u32 reg;
    const struct uart_clk_parent *const parents;
};

static struct uart_clk_comp uart_clk_comp_array[] = {
    {
        .reg = REG_CRU_TOP_CLK_TOP_LPUART_PD_CFG,
        .parents = clk_top_lpuart_pd_parents
    },
    {
        .reg = REG_SYS_ND_GLB_CLK_ND_UART_RX_CFG,
        .parents = clk_top_lpuart_pd_parents
    },
    {
        .reg = REG_CRU_TOP_CLK_TOP_UART_CFG,
        .parents = clk_top_uart_parents
    },
    {
        .reg = REG_CRU_MCU_CLK_MCU_UART0_CFG,
        .parents = clk_mcu_uart_parents
    },
    {
        .reg = REG_CRU_MCU_CLK_MCU_UART1_CFG,
        .parents = clk_mcu_uart_parents,
    }
};

struct aic_uart {
    u32 inf;
    u32 status;
    u32 base;
    u32 source_clk;
    u32 irq;
    u32 dma;
    u32 hwid;
    u32 wsize;
    u32 IRmode;
    u32 uclken;
    u32 parity;
    u32 baud;
    u32 cd;
    u32 eb_reg;
    u32 eb_bit;
    u32 rst_reg;
    u32 rst_bit;
    u32 parent_clk;
    u32 source_clk_index;
    u32 nd_parent_clk;
    u32 nd_source_clk_index;
};

/* bootloader SW log buffer enable */
#define LOG_BUF_SIZE 4096

#define SERIAL_CONSOLE_INDEX 1

struct aic_uart aic_uart[4] = {
    {
        .inf = SERIAL_INF0,
        .base = REG_BASE_TOP_TOP_UART_TX,
        .baud = 115200,
        .source_clk = SERIAL_26M_CLK,
        .cd = 0x1,
        .wsize = SERIAL_WSIZE1,
        .IRmode = SERIAL_UART,
        .uclken = SERIAL_SRC_UCLK,
        .parity = NO_PARITY,
        .eb_reg = REG_SYS_ND_GLB_ND_UART_RX_EB,
        .eb_bit = BIT_SYS_ND_GLB_ND_UART_RX_EB,
        .rst_reg = REG_SYS_ND_GLB_ND_UART_RX_SW_RST_GLB_CFG,
        .rst_bit = BIT_SYS_ND_GLB_SW_RST_GLB_ND_UART_RX,
        .parent_clk = CLK_TOP_LPUART_PD_CFG,
        .source_clk_index = 0,
        .nd_parent_clk = CLK_ND_UART_RX,
        .nd_source_clk_index = 0,
    }, {
        .inf = SERIAL_INF1,
        .base = REG_BASE_TOP_UART,
        .baud = 115200,
        .source_clk = SERIAL_96M_CLK,
        .cd = 0x1,
        .wsize = SERIAL_WSIZE1,
        .IRmode = SERIAL_UART,
        .uclken = SERIAL_SRC_UCLK,
        .parity = NO_PARITY,
        .eb_reg = REG_CKM_EN_GLB_TOP_TOP_UART_EB,
        .eb_bit = BIT_CKM_EN_GLB_TOP_TOP_UART_EB,
        .rst_reg = REG_CKM_EN_GLB_TOP_TOP_UART_SW_RST_GLB_CFG,
        .rst_bit = BIT_CKM_EN_GLB_TOP_SW_RST_GLB_TOP_UART,
        .parent_clk = CLK_TOP_UART_CFG,
        .source_clk_index = 0,
    }, {
        .inf = SERIAL_INF2,
        .base = REG_BASE_BLK_MCU_UART0,
        .baud = 115200,
        .source_clk = SERIAL_26M_CLK,
        .cd = 0x1,
        .wsize = SERIAL_WSIZE1,
        .IRmode = SERIAL_UART,
        .uclken = SERIAL_SRC_UCLK,
        .parity = NO_PARITY,
        .eb_reg = REG_CKM_EN_GLB_MCU_MCU_UART0_EB,
        .eb_bit = BIT_CKM_EN_GLB_MCU_MCU_UART0_EB,
        .rst_reg = REG_CKM_EN_GLB_MCU_MCU_UART0_SW_RST_GLB_CFG,
        .rst_bit = BIT_CKM_EN_GLB_MCU_SW_RST_GLB_MCU_UART0,
        .parent_clk = CLK_MCU_UART0_CFG,
        .source_clk_index = 1,
    }, {
        .inf = SERIAL_INF3,
        .base = REG_BASE_BLK_MCU_UART1,
        .baud = 115200,
        .source_clk = SERIAL_26M_CLK,
        .cd = 0x1,
        .wsize = SERIAL_WSIZE1,
        .IRmode = SERIAL_UART,
        .uclken = SERIAL_SRC_UCLK,
        .parity = NO_PARITY,
        .eb_reg = REG_CKM_EN_GLB_MCU_MCU_UART1_EB,
        .eb_bit = BIT_CKM_EN_GLB_MCU_MCU_UART1_EB,
        .rst_reg = REG_CKM_EN_GLB_MCU_MCU_UART1_SW_RST_GLB_CFG,
        .rst_bit = BIT_CKM_EN_GLB_MCU_SW_RST_GLB_MCU_UART1,
        .parent_clk = CLK_MCU_UART1_CFG,
        .source_clk_index = 1,
   }
};

#endif /*__SERIAL_CFG_H__*/
