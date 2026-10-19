/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: MIT
 *
 * Change Logs:
 * Date           Author        Notes
 * 2022-05-13     Meco Man      First version
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x14d80e
#define LOG_MODULE    AMOD_LVGL

#ifdef __RTTHREAD__

void lv_user_gui_init(void)
{
    extern void ui_init(void);
    ui_init();
}

#endif /* __RTTHREAD__ */
