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
    INT_AON_TMR_MCU         = BIT(0),
    INT_AON_WDT_CP_RST_REQ  = BIT(2),
    INT_AON_WDT_MCU_IRQ_REQ = BIT(3),
    INT_AUDIO_WDT_RST_REQ   = BIT(4),
    INT_AON_UART            = BIT(5),
    INT_AON_GPIO            = BIT(6),
    INT_PMIC                = BIT(7),
    INT_AON_EIPU            = BIT(8),
    INT_AON_KPC             = BIT(9),
    INT_AON_GPD             = BIT(10),
    INT_CP_PWR_UP           = BIT(12),
    INT_CP_PWR_DOWN         = BIT(13),
    INT_AUDIO_PWR_UP        = BIT(14),
    INT_AUDIO_PWR_DOWN      = BIT(15),
    INT_TOP_I2CC_PMIC_MCU   = BIT(16),
    INT_TOP_I2CC0           = BIT(17),
    INT_TOP_I2CC1           = BIT(18),
    INT_TOP_THMC            = BIT(19),
    INT_TOP_UART0           = BIT(20),
    INT_TOP_UART1           = BIT(21),
    INT_TOP_IIS0            = BIT(22),
    INT_TOP_IIS1            = BIT(23),
    INT_TOP_EFUSE           = BIT(24),
    INT_TOP_GPIO                = BIT(25),
    INT_TOP_MBOX_OUTBOX_D_MCU   = BIT(26),
    INT_TOP_MBOX_OUTBOX_MCU     = BIT(27),
    INT_TOP_MBOX_INBOX_MCU      = BIT(28),
    INT_MEM_FLASH               = BIT(29),
    INT_MEM_PSRAM               = BIT(30),
}MCU_IPU_NUM_E;

#endif /* __AP_IPU_H__ */
