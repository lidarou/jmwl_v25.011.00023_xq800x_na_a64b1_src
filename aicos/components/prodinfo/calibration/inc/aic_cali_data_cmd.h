/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file export the calibration data lock register callback interface.
 **---------------------------------------------------------------------------*/

#ifndef __AIC_CALI_DATA_CMD_H__
#define __AIC_CALI_DATA_CMD_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
typedef void (*CALI_DATA_LOCK_FUNC)(boolean lock, void *param);

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * aic_cali_data_register_hook -
 *  register callback to calibration data command,
 *  will be call it if calibration read or write.
 *
 * Parameter:
 *  @lock_func: input calibration data lock callback.
 *  @param: input callback param.
 *
 * Return:
 */
void aic_cali_data_register_hook(CALI_DATA_LOCK_FUNC lock_func, void* param);

/**
 * aic_cali_data_unregister_hook -
 *  unregister callback to calibration data command.
 *
 * Parameter:
 *  @lock_func: input calibration data lock callback.
 *
 * Return:
 */
void aic_cali_data_unregister_hook(CALI_DATA_LOCK_FUNC lock_func);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_CALI_DATA_CMD_H__ */

