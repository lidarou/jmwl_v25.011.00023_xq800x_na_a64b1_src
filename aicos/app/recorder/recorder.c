/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implements the basic functions for recorder app start/stop.
 */

/*******************************************
 *              INCLUDES
 ******************************************/
#include "recorder.h"
#include "recorder_conf.h"
#include "recorder_page_main.h"
#include "recorder_srv.h"
#ifdef WIN32
#include "recorder_simu_adpt.h"
#else
#include "aic_osa_ex.h"
#include "alv_app_mgr.h"
#endif
/*******************************************
 *              GLOBAL VARIABLES
 ******************************************/
struct recorder_service *g_recorder_srv;

/*******************************************
 *              STATIC VARIABLES
 ******************************************/
static lv_obj_t *main_obj;

/*******************************************
 *              STATIC FUNCTIONS
 ******************************************/
static void app_recorder_init(lv_obj_t *app)
{
    g_recorder_srv = app_srv_recording_init();
    if (!g_recorder_srv) {
        app_loge("recorder service init failed\n");
        return;
    }
    main_obj = app_recorder_main_create(app);
}

static void app_recorder_deinit(lv_obj_t *app)
{
    app_srv_recording_deinit(g_recorder_srv);
    lv_obj_del(main_obj);
}

static void app_recorder_focus(lv_obj_t *app)
{
    LV_UNUSED(app);
}

static void app_recorder_defocus(lv_obj_t *app)
{
    LV_UNUSED(app);
}

/*******************************************
 *              GLOBAL FUNCTIONS
 ******************************************/
void app_recorder_register(void)
{
    lv_obj_t *app = alv_am_app_create(lv_scr_act(), "Recorder");
    if (app) {
        alv_am_app_set_active(app, app_recorder_init);
        alv_am_app_set_deactive(app, app_recorder_deinit);
        alv_am_app_set_focus(app, app_recorder_focus);
        alv_am_app_set_defocus(app, app_recorder_defocus);
    } else {
        app_loge("Failed to start Recorder APP\n");
    }
}
