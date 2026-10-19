
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the api header file of the subsys.
 */
#ifndef __AIC_CTRL_H__
#define __AIC_CTRL_H__
#include <aic_boot_mode.h>

typedef enum {
    AIC_BOOTED_STATE = 0,
    AIC_ALIVE_STATE,
    AIC_ASSERT_STATE,
    AIC_SHUTDOWN_STATE,
} AIC_STATE;

typedef int32_t (*set_config_callback)(void *p_param, uint32_t size);

/**
 * aic_ctrl_set_abnormal_pin - .change the abnormal pin status.
 *
 * @active: 0 -- init status, 1 - abnormal notify.
 */
void aic_ctrl_set_abnormal_pin(bool active);

/**
 * aic_ctrl_state_notify - notify the state of aic to extern ap.
 *
 * @state: the state of aic.
 */
void aic_ctrl_state_notify(AIC_STATE state);

/**
 * aic_ctrl_state_notify - notify the state of aic to extern ap.
 *
 * @state: the state of aic.
 */
void aic_ctrl_mode_notify(BOOT_MODE_E state);

/**
 * aic_ctrl_str_notify - notify the specified str to extern ap.
 *
 * @state: the state of aic.
 */
void aic_ctrl_str_notify(const char *str);

/**
 * aic_ctrl_set_config_register - register set config callback.
 *
 * @p_module: the string of module.
 * @_cfg_callback: the set config callback.
 */
int32_t aic_ctrl_set_config_register(char *p_module, set_config_callback _cfg_callback);

/**
 * aic_ctrl_set_config_unregister - unregister set config callback.
 *
 * @client_handle: the registed handle.
 */
int32_t aic_ctrl_set_config_unregister(int32_t client_handle);

#endif /* __AIC_CTRL_H__ */

