/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines sys dump configuration.
 */

#ifndef __SYS_DUMP_CFG_H__
#define __SYS_DUMP_CFG_H__

#include <aonram_mem_config.h>
#include <flash_mem_config.h>
#include <psram_mem_config.h>
#include <pinlist.h>

#ifdef CONFIG_SYS_DUMP_INIT_UART_PIN
#define DUMP_UART_TX_PIN        REG_IOMUX_CAN_TXD
#define DUMP_UART_TX_VALUE      BIT_FUNC_SEL(1)

#define DUMP_UART_TX_PAD_PIN    REG_PG10_PAD_CTRL_CAN_TXD
#define DUMP_UART_TX_PAD_VALUE  (BIT_PAD_DRV(1)|BIT_PAD_PX|BIT_PAD_SLP_MCU|BIT_PAD_SLP_WPU|BIT_PAD_SLP_Z)

#define DUMP_UART_RX_PIN        REG_IOMUX_CAN_RXD
#define DUMP_UART_RX_VALUE      BIT_FUNC_SEL(1)

#define DUMP_UART_RX_PAD_PIN    REG_PG10_PAD_CTRL_CAN_RXD
#define DUMP_UART_RX_PAD_VALUE  (BIT_PAD_DRV(1)|BIT_PAD_WPU|BIT_PAD_SLP_MCU|BIT_PAD_SLP_WPU|BIT_PAD_SLP_Z)
#endif

static const struct sys_dump_cfg_info s_sys_dump_cfg[] = {
    {
        .type = DUMP_MEM,
        .address = AONRAM_REGION_BASE,
        .length = AONRAM_REGION_SIZE,
    },
    {
        .type = DUMP_MEM,
        .address = PSRAM_REGION_BASE,
        .length = PSRAM_REGION_SIZE,
    },
    {
        .type = DUMP_MEM,
        .address = FLASH0_BASE,
        .length = FLASH0_TOTAL_SIZE,
    },
};

static const struct sys_dump_cfg_info *sys_dump_get_cfg(void)
{
    return s_sys_dump_cfg;
}

static int sys_dump_get_cfg_num(void)
{
    return sizeof(s_sys_dump_cfg)/sizeof(struct sys_dump_cfg_info);
}

#endif /* __SYS_DUMP_CFG_H__ */
