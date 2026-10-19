/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#ifndef __SERIAL_CFG_H__
#define __SERIAL_CFG_H__

#include <reg_base.h>
#include <mcu_glb.h>
#include <top_glb.h>
#include <aon_glb.h>
#include "inttypes.h"

/* mode */
#define SERIAL_WSIZE1OR2    0
#define SERIAL_WSIZE1       1
#define SERIAL_WSIZE2       2
#define SERIAL_WSIZE3       3

#define SERIAL_UART         0
#define SERIAL_IRDA         1

#define SERIAL_SRC_PCLK     0
#define SERIAL_SRC_UCLK     1

enum PAR{
    EVEN_PARITY,
    ODD_PARITY,
    FORCE_0_PARITY,
    FORCE_1_PARITY,
    NO_PARITY,
};

struct aic_uart {
    u32 index;
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
};


/* bootloader SW log buffer enable */
#define LOG_BUF_SIZE 4096

#define SERIAL_SOURCE_CLK   26000000
#define SERIAL_CONSOLE_INDEX 4

struct aic_uart aic_uart[6] = {
    {
        .index = 0,
        .base = REG_BASE_AP_UARTC0,
        .baud = 115200,
        .source_clk = SERIAL_SOURCE_CLK,
        .cd = 0x1,
        .wsize = SERIAL_WSIZE1,
        .IRmode = SERIAL_UART,
        .uclken = SERIAL_SRC_UCLK,
        .parity = NO_PARITY,
        .eb_reg = REG_MCU_GLB_UART_0_EB,
        .eb_bit = BIT_MCU_GLB_UART0_EB,
        .rst_reg = REG_MCU_GLB_SW_RST_GLB_CLK_MCU_UART0,
        .rst_bit = BIT_MCU_GLB_SW_RST_GLB_CLK_MCU_UART0,
    }, {
        .index = 1,
        .base = REG_BASE_AP_UARTC1,
        .baud = 115200,
        .source_clk = SERIAL_SOURCE_CLK,
        .cd = 0x1,
        .wsize = SERIAL_WSIZE1,
        .IRmode = SERIAL_UART,
        .uclken = SERIAL_SRC_UCLK,
        .parity = NO_PARITY,
        .eb_reg = REG_MCU_GLB_UART_1_EB,
        .eb_bit = BIT_MCU_GLB_UART1_EB,
        .rst_reg = REG_MCU_GLB_SW_RST_GLB_CLK_MCU_UART1,
        .rst_bit = BIT_MCU_GLB_SW_RST_GLB_CLK_MCU_UART1,
    }, {
        .index = 2,
        .base = REG_BASE_AP_UARTC2,
        .baud = 115200,
        .source_clk = SERIAL_SOURCE_CLK,
        .cd = 0x1,
        .wsize = SERIAL_WSIZE1,
        .IRmode = SERIAL_UART,
        .uclken = SERIAL_SRC_UCLK,
        .parity = NO_PARITY,
        .eb_reg = REG_MCU_GLB_UART_2_EB,
        .eb_bit = BIT_MCU_GLB_UART2_EB,
        .rst_reg = REG_MCU_GLB_SW_RST_GLB_CLK_MCU_UART2,
        .rst_bit = BIT_MCU_GLB_SW_RST_GLB_CLK_MCU_UART2,
    }, {
        .index = 3,
        .base = REG_BASE_AON_UART,//interface 0
        .baud = 115200,
        .source_clk = SERIAL_SOURCE_CLK,
        .cd = 0x1,
        .wsize = SERIAL_WSIZE1,
        .IRmode = SERIAL_UART,
        .uclken = SERIAL_SRC_UCLK,
        .parity = NO_PARITY,
        .eb_reg = REG_AON_GLB_UART_EB,
        .eb_bit = BIT_AON_GLB_UART_EB,
        .rst_reg = REG_TOP_GLB_SW_RST_AON_UART,
        .rst_bit = BIT_TOP_GLB_SW_RST_GLB_CLK_AON_UART,
   }, {
        .index = 4,
        .base = REG_BASE_TOP_UART0,//interface 1
        .baud = 115200,
        .source_clk = SERIAL_SOURCE_CLK,
        .cd = 0x1,
        .wsize = SERIAL_WSIZE1,
        .IRmode = SERIAL_UART,
        .uclken = SERIAL_SRC_UCLK,
        .parity = NO_PARITY,
        .eb_reg = REG_TOP_GLB_UART0_EB,
        .eb_bit = BIT_TOP_GLB_UART0_EB,
        .rst_reg = REG_TOP_GLB_SW_RST_TOP_UART0,
        .rst_bit = BIT_TOP_GLB_SW_RST_GLB_CLK_TOP_UART0,

    },{
        .index = 5,
        .base = REG_BASE_TOP_UART1,//interface 2
        .baud = 115200,
        .source_clk = SERIAL_SOURCE_CLK,
        .cd = 0x1,
        .wsize = SERIAL_WSIZE1,
        .IRmode = SERIAL_UART,
        .uclken = SERIAL_SRC_UCLK,
        .parity = NO_PARITY,
        .eb_reg = REG_TOP_GLB_UART1_EB,
        .eb_bit = BIT_TOP_GLB_UART1_EB,
        .rst_reg = REG_TOP_GLB_SW_RST_TOP_UART1,
        .rst_bit = BIT_TOP_GLB_SW_RST_GLB_CLK_TOP_UART1,

    },
};

#endif /*__SERIAL_CFG_H__*/
