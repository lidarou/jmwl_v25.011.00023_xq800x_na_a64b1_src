/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines pwr key driver api.
 *    The pwr key api will be called by aicos user functions.
 */

#ifndef __PWR_KEY_H__
#define __PWR_KEY_H__

typedef enum {
    AIC_PWR_KEYDOWN = 1,
    AIC_PWR_KEYUP,
#ifdef CONFIG_FP
    AIC_PWR_KEYLONG,
#endif
    AIC_PWR_KEY_MAX
} AIC_PWR_KEY_STATUS;

typedef int32_t (*pwr_key_callback)(void *para, uint32_t size);

/**
 * register_pwr_key_monitor - register power key status change monitor
 * @monitor_name: user defined monitor name.
 * @callback: pwr press status change call monitor callback.
 */
int register_pwr_key_monitor(char *monitor_name, pwr_key_callback callback);

/**
 * unregister_pwr_key_monitor - unregister power key status change monitor.
 * @monitor_name: user defined monitor name.
 */
int unregister_pwr_key_monitor(char *monitor_name);

/**
 * pwrkey_sd_thold_cfg - long press power button auto shutdown timing threshold config.
 * @shutdown_thold_ms: long press pwr key threshold unit is ms, 0xFFFF never trigger auto shutdown.
 */
void pwrkey_sd_thold_cfg(u32 shutdown_thold_ms);

#endif /* __PWR_KEY_H__ */
