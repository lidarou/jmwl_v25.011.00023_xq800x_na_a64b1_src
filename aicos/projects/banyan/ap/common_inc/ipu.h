/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of ap ipu module.
 *   It includes all the ap ipu common function api.
*/

#ifndef __AP_IPU_H__
#define __AP_IPU_H__

typedef enum _mcu_ipu_num_e
{
    INT_REQ_TOP_WDT_CP_RST_REQ    = BIT(0),
    INT_REQ_TOP_WDT_MCU           = BIT(1),
    INT_REQ_PMIC                  = BIT(2),
    INT_REQ_AON_EIPU              = BIT(3),
    INT_REQ_AON_CP_PWRUP          = BIT(4),
    INT_REQ_AON_CP_PWRDW          = BIT(5),
    INT_REQ_TOP_I2CC_PMIC_MCU     = BIT(6),
    INT_REQ_TOP_I2CC0             = BIT(7),
    INT_REQ_TOP_THMC              = BIT(8),
    INT_REQ_TOP_UART0             = BIT(9),
    INT_REQ_TOP_IIS0              = BIT(10),
    INT_REQ_TOP_EFUSE             = BIT(11),
    INT_REQ_TOP_GPIO              = BIT(12),
    INT_REQ_TOP_MBOX_OUTBOX_D_MCU = BIT(13),
    INT_REQ_TOP_MBOX_OUTBOX_MCU   = BIT(14),
    INT_REQ_TOP_MBOX_INBOX_MCU    = BIT(15),
    INT_REQ_MEM_FLASH             = BIT(16),
}MCU_IPU_NUM_E;

typedef enum _mcu_nd_ipu_num_e
{
    INT_REQ_ND_LPUART             = BIT(0),
    INT_REQ_ND_TMR_MCU            = BIT(1),
    INT_REQ_ND_EIPU               = BIT(2),
    INT_REQ_ND_PMIC               = BIT(3),
}MCU_ND_IPU_NUM_E;

#endif /* __AP_IPU_H__ */
