/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  This file defines display driver api.
 */

#ifndef __DISPLAY_H__
#define __DISPLAY_H__
#include <aic_type.h>
/**
 * display_refresh - flip framebuffer data to panel as system config mode.(async or sync)
 * @fb_addr: framebuffer address which to be flipped to panel display.
 * no return.
 */
void display_refresh(u32 fb_addr);

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

#endif /* __DISPLAY_H__ */
