/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file export the calibration command base operation struct and interface.
 **---------------------------------------------------------------------------*/

#ifndef __AIC_CALI_CMD_H__
#define __AIC_CALI_CMD_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_boot_mode.h>

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
typedef void (*CALI_FUNC)(BOOT_MODE_E mode, void *param);

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * aic_cali_register_hook -
 *  register callback to calibration, will be call it if enter calibration.
 *
 * Parameter:
 *  @func: input calibration callback.
 *  @param: input callback param.
 *
 * Return:
 */
void aic_cali_register_hook(CALI_FUNC func, void* param);

/**
 * aic_cali_unregister_hook - unregister callback to calibration.
 *
 * Parameter:
 *  @func: input calibration callback.
 *
 * Return:
 */
void aic_cali_unregister_hook(CALI_FUNC func);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_CALI_CMD_H__ */

