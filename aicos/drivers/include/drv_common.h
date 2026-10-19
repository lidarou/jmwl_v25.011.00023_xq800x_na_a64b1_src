/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines COMMON elements of drivers.
 *    The file will be included by every interface header file.
 */

#ifndef __DRV_COMMON_H__
#define __DRV_COMMON_H__

#include <aic_type.h>
#include <aic_errno.h>

#define AIC_DRIVER_VERSION(major,minor) (((major) << 8) | (minor))

/**
 * struct aic_driver_version - hardware and driver version
 * @hw_ver: hardware module version
 * @drv_ver: software driver version
 */
struct aic_driver_version {
  uint16_t hw_ver;
  uint16_t drv_ver;
};

/**
 * common driver return codes.
 * @AIC_DRIVER_OK: operation success
 * @AIC_DRIVER_ERROR: unspecified error
 * @AIC_DRIVER_ERROR_BUSY: driver is busy
 * @AIC_DRIVER_ERROR_TIMEOUT: timeout
 * @AIC_DRIVER_ERROR_UNSUPPORTED: operation not support
 * @AIC_DRIVER_ERROR_PARAMETER: parameter error
 * @AIC_DRIVER_ERROR_SPECIFIC: start of driver specificed errors
 */
#define AIC_DRIVER_OK                 0
#define AIC_DRIVER_ERROR             -1
#define AIC_DRIVER_ERROR_BUSY        -2
#define AIC_DRIVER_ERROR_TIMEOUT     -3
#define AIC_DRIVER_ERROR_UNSUPPORTED -4
#define AIC_DRIVER_ERROR_PARAMETER   -5
#define AIC_DRIVER_ERROR_SPECIFIC    -6

#endif /* __DRV_COMMON_H__ */

