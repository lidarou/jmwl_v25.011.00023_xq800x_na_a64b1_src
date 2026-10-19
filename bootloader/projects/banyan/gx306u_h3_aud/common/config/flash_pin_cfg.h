/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the extend flash pin cfg.
 */

#ifndef __FLASH_PIN_CFG_H__
#define __FLASH_PIN_CFG_H__

#define FLASH_PIN_GROUP_INDEX   (0)
#define FLASH_PIN_DRV           (4)

#define FLASH_BUK0_BOUND_REG        (0x4228)
#define FLASH_BUK0_SLP_BOUND        (0x3 << 4)

#define FLASH_LDO_CFG_UPDATE_REG    (0x424C)
#define FLASH_LDO_UPDATE            (0x1 << 0)

#endif /* __FLASH_PIN_CFG_H__ */
