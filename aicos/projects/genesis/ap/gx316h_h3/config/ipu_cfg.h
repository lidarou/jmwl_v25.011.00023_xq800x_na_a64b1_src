/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __IPU_BOARD_CFG_H__
#define __IPU_BOARD_CFG_H__

#include <reg_base.h>
#include <irq.h>
#include <top_glb.h>
#include <aon_glb.h>
#include <mcu_glb.h>
#include <ipu.h>
#include <ipu_inc.h>

struct aic_eipu_board_cfg {
    IPU_TYPE_E type;
    u32 base;
    u32 aon_eb;
    u32 eb_bits;
};

static struct aic_eipu_board_cfg ipu_bd_cfg[1] = {
    {
        .type = IPU_MCU,
        .base = REG_BASE_AON_IPU_MCU,
        .aon_eb = REG_AON_GLB_IPU_MCU_EB,
        .eb_bits =
            INT_AON_TMR_MCU |
            INT_AON_WDT_CP_RST_REQ |
            INT_AON_WDT_MCU_IRQ_REQ |
            INT_AUDIO_WDT_RST_REQ |
            INT_AON_UART |
            INT_AON_GPIO |
            INT_PMIC |
            INT_AON_EIPU |
            INT_AON_KPC |
            INT_AON_GPD |
            INT_TOP_THMC |
            INT_TOP_UART0 |
            INT_TOP_UART1 |
            INT_TOP_GPIO |
            INT_TOP_MBOX_OUTBOX_D_MCU |
            INT_TOP_MBOX_OUTBOX_MCU |
            INT_TOP_MBOX_INBOX_MCU,
        },
};

#endif /* __IPU_BOARD_CFG_H__ */

