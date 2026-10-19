/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the basic Application Interface (API) of file
 *    browser app flow(fbaf).  All app prototypes and data structure
 *    definitions are defined in this file.
 */

#ifndef __APP_FOTA_FLOW_H__
#define __APP_FOTA_FLOW_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/
#include <lvgl.h>

/*******************************************
 *              GLOBAL PROTOTYPES
 ******************************************/
enum fota_app_status {
    app_fota_init = 0,
    app_fota_version_checking,
    app_fota_version_checked,
    app_fota_downloading,
    app_fota_need_reboot,
};

enum fota_app_event {
    fae_state_change = 0,
    fae_state_error,
};

/*******************************************
 *              GLOBAL PROTOTYPES
 ******************************************/
/**
 * flow_fota_init - init fota app flow status
 * @app: [IN] pointer to app obj
 *
 * Return: the current page in show
 */
void flow_fota_init(lv_obj_t *app);

/**
 * flow_fota_destory - destroy fota app flow status
 * @app: [IN] pointer to app obj
 *
 * Return: NA
 */
void flow_fota_destory(lv_obj_t *app);

/**
 * flow_fota_suspend - suspend the app
 * @app: [IN] pointer to app obj
 *
 * Return: NA
 */
void flow_fota_suspend(lv_obj_t *app);

/**
 * flow_fota_resume - resume the app
 * @app: [IN] pointer to app obj
 *
 * Return: NA
 */
void flow_fota_resume(lv_obj_t *app);

/**
 * @brief let fota app back to last app
 */
void app_fota_back();

/**
 * @brief get fota status from app_service
 */
enum fota_app_status app_fota_get_status();

/**
 * @brief check fota is started
 * @return true for started
 */
bool app_fota_is_started();

/**
 * @brief start fota process
 * @return  0   sucess
 *         <0   error code
 */
int app_fota_start_update();

/**
 * @brief stop fota process
 * @return  0   sucess
 *         <0   error code
 */
int app_fota_stop_update();

/**
 * @brief trigger system to reboot to bootloader for fota
 * @return  0   sucess
 *         <0   error code
 */
int app_fota_reboot_to_update();

/**
 * @brief get current version of firmware
 * @return  0   sucess
 *         <0   error code
 */
const char *app_fota_get_cur_version();

/**
 * @brief get update version of firmware
 * @return  0   sucess
 *         <0   error code
 */
const char *app_fota_get_update_version();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __APP_FOTA_FLOW_H__ */