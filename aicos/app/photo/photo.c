/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is enter of photo app
 */

/*********************
 *      INCLUDES
 *********************/
#include "photo.h"
#include "photo_flow.h"
#include "aic_osa_ex.h"
#include "alv_app_mgr.h"

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
extern void ui_photo_main_init(void);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void photo_app_enter(lv_obj_t *app)
{
    ui_photo_main_init();
    flow_photo_enter(0);
}

static void photo_app_leave(lv_obj_t *app)
{
    flow_photo_leave(0);
}

static void photo_app_focus(lv_obj_t *app)
{
}

static void photo_app_defocus(lv_obj_t *app)
{
}
/**********************
 *   PUBLIC FUNCTIONS
 **********************/

/**
 * app_photo_init() - init of photo app
 * 
 * Return: pointer to photo app object.
 */
void app_photo_init(void)
{
    lv_obj_t *app = alv_am_app_create(NULL, "photo");
    if (!app) {
        app_loge("%s: app create failed\n", __func__);
        return;
    }
    alv_am_app_set_active(app, photo_app_enter);
    alv_am_app_set_deactive(app, photo_app_leave);
    alv_am_app_set_focus(app, photo_app_focus);
    alv_am_app_set_defocus(app, photo_app_defocus);
}
