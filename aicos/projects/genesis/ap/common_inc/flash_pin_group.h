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

#ifndef __FLASH_PIN_GROUP_H__
#define __FLASH_PIN_GROUP_H__

#include <flash_config.h>
#include <qspi_inc.h>

#if (QSPI_NOR_DEVICE_NUM == 2)
struct flash_pin_cfg extflash_pin_cfg = {
    .pin_cfg = NULL,
    .pin_num = 0,
};
#endif

#endif /* __FLASH_PIN_GROUP_H__ */
