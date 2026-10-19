/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of usb.
 */

#ifndef __USB_H__
#define __USB_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "usb_flow.h"
#include "ui/win_usb/ui.h"
#include "ui/win_charging/ui.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
extern const usb_flow_t g_usb_flow;

/**
* app_usb_init
* Return: .
* Description: when app init,call this function to initial usb app .
*/
#ifdef CONFIG_APP_USB
static void app_usb_init(void)
{
    /* Create APP for usb */
    lv_obj_t * app_usb = alv_am_app_create(lv_scr_act(), "usb");

    /* Create usb win and set app first win */
    lv_obj_t *win_usb = alv_am_win_create(app_usb, "usb win");
    alv_am_win_set_active(win_usb, usb_win_active);
    alv_am_win_set_deactive(win_usb, usb_win_deactive);
    alv_am_win_set_focus(win_usb, usb_win_focus);

    lv_obj_t *win_charging = alv_am_win_create(app_usb, "charging_win");
    alv_am_win_set_active(win_charging, charging_win_active);
    alv_am_win_set_deactive(win_charging, charging_win_deactive);

    alv_am_app_set_first_win(app_usb, win_usb);

    /* register usb ui update callbacks */
    usb_flow_register_ui_actions(&g_usb_flow);

    /* register usb flow process */
    usb_flow_init();
}
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __USB_H__ */
