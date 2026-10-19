// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.1.1
// LVGL VERSION: 8.2.0
// PROJECT: mainmenu

#ifndef _BOOTAINIMATION_H_
#define _BOOTAINIMATION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#ifdef CONFIG_BOOT_ANIMATION
LV_IMG_DECLARE(power_on);    // power_on.gif
#else
#ifdef CONFIG_FP
LV_IMG_DECLARE(ui_img_logo_png);
#else
LV_IMG_DECLARE(power_on_picture);// power_on_picture.png
#endif
#endif
#ifdef CONFIG_FP
LV_IMG_DECLARE(charging);
#else
LV_IMG_DECLARE(charge);    // charge.gif
#endif
LV_IMG_DECLARE(hehe);    // hehe.gif

int show_boot_animation(void);

int show_charge_animation(void);
int show_charge_picture(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
