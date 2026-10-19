/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines display driver api.
 *    The display api will be called by aicos user functions.
 */

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

typedef enum {
    AIC_BL_TURN_ON = 1,
    AIC_BL_TURN_OFF,
    AIC_BL_TYPE_MAX
} AIC_BL_SWITCH;

typedef void (*bl_switch_func)(AIC_BL_SWITCH bl_status);

/**
 * display_refresh - flip framebuffer data to panel as system config mode.(async or sync)
 * @fb_addr: framebuffer address which to be flipped to panel display.
 * no return.
 */
void display_refresh(u32 fb_addr);

/**
 * display_refresh_sync - flip framebuffer data to panel as sync mode, no matter what system config mode.
 * @fb_addr: framebuffer address which to be flipped to panel display.
 * non return.
 */
void display_refresh_sync(u32 fb_addr);

/**
 * display_suspend - take display ip & lcd panel into sleep, to save power
 * non parameters.
 * non return.
 */
void display_suspend(void);

/**
 * display_resume - take display ip & lcd panel from sleep, return normal work
 * non parameters.
 * non return.
 */
void display_resume(void);

/**
 * aic_bl_brightness_set - set backlight brightness
 * @brt_perc: percentage of maximum brightness.
 * return 0 if set success, else fail.
 */
int aic_bl_brightness_set(u32 brt_perc);

/**
 * aic_bl_brightness_get - get backlight brightness
 * @brt_perc: point to percentage of maximum brightness.
 * return 0 if set success, else fail.
 */
int aic_bl_brightness_get(u32* brt_perc);

/**
 * broadcast_backlight_switch - broadcast backlight switch status.
 * @bl_status: aic backlight switch status.
 */
void broadcast_backlight_switch(AIC_BL_SWITCH bl_status);

/**
 * register_backlight_switch_monitor - register backlight status change monitor
 * @monitor_name: user defined monitor name.
 * @callback: backlight status change call monitor callback.
 */
int register_backlight_switch_monitor(char *monitor_name, bl_switch_func callback);

/**
 * unregister_backlight_switch_monitor - unregister backlight status change monitor.
 * @monitor_name: user defined monitor name.
 */
int unregister_backlight_switch_monitor(char *monitor_name);
#endif /* __DISPLAY_H__ */
