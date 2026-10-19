
/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines misc config parameters.
 */

#ifndef __MISC_CONFIG_H__
#define __MISC_CONFIG_H__

#include <aic_md_id.h>

#define MD_BLOCK_SIZE               0x2000
#define MD_SECTOR_SIZE              0x200
#define MD_FLASH_SECTOR_SIZE        0x1000

/**----------------------------- misc -----------------------------------------*
 |      name                 |      block number
 |-------------------------------------------------|
 |protect                    |          2
 |-------------------------------------------------|
 |normal                     |          2
 |-------------------------------------------------|
 |user                       |          2
 |-------------------------------------------------|
 |spare                      |          1
 |-------------------------------------------------|
 |total                      |          7
 **---------------------------------------------------------------------------*/
#define MD_CALI_BLOCK_NUM           0x1
#define MD_NORMAL_BLOCK_NUM         0x2
#define MD_USER_BLOCK_NUM           0x2

#define MD_DL_DEVICE_NAME           "md"
#define MD_DL_SIZE                  0x2000

#define MD_CALIBRATION_DEVICE_NAME  "mdcali"
#define MD_NORMAL_DEVICE_NAME       "mdnorm"
#define MD_USER_DEVICE_NAME         "mduser"

/**
 * whitelist misc id: if defined and this id is calibration item,
 * this id will be support write to calibration partition in normal mode.
 */
u16 md_whitelist_id[] = {
    MD_ID_SN0,
    MD_ID_SN1,
    MD_ID_MAC
};

#endif /* __MISC_CONFIG_H__ */

