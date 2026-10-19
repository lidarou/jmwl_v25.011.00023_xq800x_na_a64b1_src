/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/****************************************/
/* This file is manually added and there is no part generated automatically by squareline studio. */
 /****************************************/

/*********************
 *      INCLUDES
 *********************/
#include "alv_app_mgr.h"
#include "statusbar.h"
#include "ui.h"
#include "usb_flow.h"

/*********************
 *      DEFINES
 *********************/
#define SHOW_CHARGING_TIMEOUT (3000)

/**********************
 *   STATIC VARIABLES
 **********************/
static lv_timer_t *g_charging_timer = NULL;

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void charging_timer_handle(lv_timer_t * t)
{
    alv_am_win_exit(NULL, "charging_win", NULL);
}
/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void charging_win_active(lv_obj_t *win, void *p_param)
{
    uint8_t bat_capacity = 0;
    /* create charging controls */
    ui_charging_init();
    /*refresh charging display*/
    bat_capacity = usb_flow_get_bat_capacity();
    lv_arc_set_value(ui_ArcCharging, bat_capacity);
    lv_label_set_text_fmt(ui_LblCharging, "%d%%", bat_capacity);

    /* NOT show statusbar */

    /*create timer*/
    if(g_charging_timer == NULL) {
        g_charging_timer = lv_timer_create(charging_timer_handle, SHOW_CHARGING_TIMEOUT, NULL);
    }
}
void charging_win_deactive(lv_obj_t *win, void *p_param)
{
    lv_obj_del(ui_Charging);
    lv_timer_del(g_charging_timer);
    g_charging_timer = NULL;

    alv_am_app_exit(NULL, "usb", NULL);
}
