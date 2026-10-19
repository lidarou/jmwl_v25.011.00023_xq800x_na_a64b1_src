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
#include "alv_app_mgr.h"
#include "filebrowser.h"
#include "filebrowser_flow.h"
#include "filebrowser_service.h"

/*******************************************
 *              GLOBAL FUNCTIONS
 ******************************************/
/**
 * register_app_filebrowser - register file browser app into app manager
 *
 * Return: NA
 */
void register_app_filebrowser(void)
{
    lv_obj_t *app = alv_am_app_create(NULL, FB_APP_NAME);
    if (app) {
#ifndef WIN32
        app_srv_fb_init();
#endif
        alv_am_app_set_active(app, flow_fb_init);
        alv_am_app_set_deactive(app, flow_fb_destory);
        alv_am_app_set_focus(app, flow_fb_resume);
        alv_am_app_set_defocus(app, flow_fb_suspend);
    } else {
        app_loge("fb_app_entry: failed to create app");
    }
}
