/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * Export the the nvram ap initialize Interface (API).
 */

#ifndef __AIC_NVRAM_AP_H__
#define __AIC_NVRAM_AP_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Function Prototypes
 **---------------------------------------------------------------------------*/
/**
 * aic_nvram_ap_initialize - nvram ap initialize.
 *
 * Return:
 *
 */
void aic_nvram_ap_initialize(void);

/**
 * nvram_ap_load_data - load device data to psram.
 *
 * Parameter:
 *
 * Return:
 *
 */
void nvram_ap_load_data(void);

/**
 * aic_nvram_ap_restore_factory - restore nvram factory setting.
 *
 * Parameter:
 *
 * Return:
 *
 */
void aic_nvram_ap_restore_factory(void);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_NVRAM_AP_H__ */

