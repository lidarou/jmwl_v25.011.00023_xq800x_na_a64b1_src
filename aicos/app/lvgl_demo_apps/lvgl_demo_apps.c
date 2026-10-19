/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file launch lvgl demo apps for aicos
 */
#include <aic_errno.h>
#include <aic_init.h>
#include <lvgl.h>
#include <tx_api.h>

void lv_demo_app_init(void)
{
#ifdef CONFIG_LV_USE_DEMO_STRESS
    lv_demo_stress();
#elif CONFIG_LV_USE_DEMO_BENCHMARK
extern void lv_demo_benchmark(void);
    lv_demo_benchmark();
#elif CONFIG_LV_USE_DEMO_KEYPAD_AND_ENCODER
    lv_demo_keypad_encoder();
#elif CONFIG_LV_USE_DEMO_MUSIC
    lv_demo_music();
#elif CONFIG_LV_USE_DEMO_WIDGETS
    lv_demo_widgets();
#endif
}

