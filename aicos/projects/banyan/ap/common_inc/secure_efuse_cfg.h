/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This is secure pubkey efuse config file.
 */

#ifndef __PUBKEY_HASH_CFG_H__
#define __PUBKEY_HASH_CFG_H__

#define PUBKEY_HASH_BLK_IDX         23
#define PUBKEY_HASH_BIT_IDX         23
#define PUBKEY_HASH_BITS_LENGTH     256

#define SEC_BOOT_EN_BLK     23
#define SEC_BOOT_EN_BIT     2

#define BONDING_OPTION_BLK  20

/* Bonding option block20 bitmap */
#define BONDING_OPTION_BLK_ENABLE_GLB           BIT(30)
#define BONDING_OPTION_BLK_DIS_CAN_TMR          BIT(12)
#define BONDING_OPTION_BLK_DIS_CAN              BIT(11)
#define BONDING_OPTION_BLK_DIS_MSRC1            BIT(10)
#define BONDING_OPTION_BLK_DIS_AONPD            BIT(9)
#define BONDING_OPTION_BLK_DIS_VOLTE            BIT(8)
#define BONDING_OPTION_BLK_DIS_IIS_DUAL         BIT(7)
#define BONDING_OPTION_BLK_DIS_SSP_IIS          BIT(6)
#define BONDING_OPTION_BLK_DIS_MCU307           BIT(5)
#define BONDING_OPTION_BLK_DIS_MCU356           BIT(4)
#define BONDING_OPTION_BLK_DIS_MCU384           BIT(3)
#define BONDING_OPTION_BLK_DIS_MCUDSP           BIT(2)
#define BONDING_OPTION_BLK_DIS_TESTMOD          BIT(1)
#define BONDING_OPTION_BLK_DIS_JTAG             BIT(0)

#endif