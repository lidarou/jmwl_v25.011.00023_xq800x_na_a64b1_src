/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implements the functionalities of file browser(FB) app.
 */

/*******************************************
 *              INCLUDES
 ******************************************/
#include "app_fota.h"
#include "app_fota_flow.h"
#include <alv_app_mgr.h>

/*******************************************
 *              GLOBAL FUNCTIONS
 ******************************************/
/**
 * register_app_fota - register fota app into app manager
 *
 * Return: NA
 */
void register_app_fota(void)
{
    lv_obj_t *app = alv_am_app_create(lv_scr_act(), FOTA_APP_NAME);
    if (app) {
        alv_am_app_set_active(app, flow_fota_init);
        alv_am_app_set_deactive(app, flow_fota_destory);
        alv_am_app_set_focus(app, flow_fota_resume);
        alv_am_app_set_defocus(app, flow_fota_suspend);
    } else {
        app_loge("register_app_fota: failed to create app");
    }
}
