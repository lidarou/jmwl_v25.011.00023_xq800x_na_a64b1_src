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
#include <ckm_en_glb_aon.h>

struct aic_eipu_board_cfg {
    IPU_TYPE_E type;
    u32 base;
    u32 aon_eb;
    u32 eb_bits;
};

static struct aic_eipu_board_cfg ipu_bd_cfg[2] = {
    {
        .type = IPU_MCU,
        .base = REG_BASE_TOP_IPU_MCU,
        .aon_eb = REG_CKM_EN_GLB_AON_AON_APB_IPU_MCU_EB,
        .eb_bits =
            INT_REQ_TOP_WDT_CP_RST_REQ    |
            INT_REQ_TOP_WDT_MCU           |
            INT_REQ_PMIC                  |
            INT_REQ_AON_EIPU              |
            INT_REQ_AON_CP_PWRUP          |
            INT_REQ_TOP_I2CC_PMIC_MCU     |
            INT_REQ_TOP_I2CC0             |
            INT_REQ_TOP_THMC              |
            INT_REQ_TOP_UART0             |
            INT_REQ_TOP_IIS0              |
            INT_REQ_TOP_EFUSE             |
            INT_REQ_TOP_GPIO              |
            INT_REQ_TOP_MBOX_OUTBOX_D_MCU |
            INT_REQ_TOP_MBOX_OUTBOX_MCU   |
            INT_REQ_TOP_MBOX_INBOX_MCU    |
            INT_REQ_MEM_FLASH,

    },

    {
        .type = IPU_ND,
        .base = REG_BASE_TOP_ND_IPU,
        .aon_eb = 0,
        .eb_bits =
            INT_REQ_ND_LPUART             |
            INT_REQ_ND_TMR_MCU            |
            INT_REQ_ND_EIPU               |
            INT_REQ_ND_PMIC,
    },
};

#endif /* __IPU_BOARD_CFG_H__ */

