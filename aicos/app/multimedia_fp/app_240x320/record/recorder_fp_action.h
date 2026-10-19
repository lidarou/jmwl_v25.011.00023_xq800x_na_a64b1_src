

/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file declare the main page create API.
 */
#ifndef __RECORDER_FP_ACTION_H__
#define __RECORDER_FP_ACTION_H__

#ifdef __cplusplus
extern "C"
{
#endif

    /*******************************************
     *              INCLUDES
     ******************************************/
#include "recorder_fp_conf.h"
#include "recorder_fp_srv.h"

    /*******************************************
     *              GLOBAL PROTOTYPES
     ******************************************/
    uint32_t is_recorder_folder_empty();
    void app_recorder_resume(uint8_t * name);
    void app_recorder_pause(void);
    void app_recorder_stop(void);
    void app_recorder_play();

    void app_recorder_set_play_path(char * path);
    void app_recorder_play_pause();

    u32 app_recorder_getvol();
    u32 app_recorder_setvol(u32 level);
    u32 app_recorder_getplaytime();
    uint32_t app_recorder_get_play_status();


    typedef int32_t(*recorder_play_status_update_func_callback) (void * p_param,uint32_t event); // typedef srv_func_callback

    void app_recorder_status_set_func_callback(recorder_play_status_update_func_callback fun,void * p_param);
    void app_recorder_service_notify_cb(uint32_t event);
    uint32_t app_recorder_get_record_status();
    void app_recorder_call_resume(uint8_t * name);
    u32 app_recorder_get_duration();
    int32_t fp_app_audio_get_file_duration(char * p_file_name,uint32_t * p_duration);
    void app_recorder_play_pause_action();
    int32_t fp_app_audio_get_amr_duration(char * p_file_name,uint32_t * p_duration);
    void app_recorder_stop_action(void);
    void app_recorder_play_pause_path(uint8_t * path);
    void app_recorder_play_action();
    void app_recorder_pause_action(void);
    void app_recorder_resume_action(uint8_t * name);
    void app_recorder_call_resume_action(uint8_t * name);
    void app_recorder_play_pause_path_action(uint8_t * path);

#ifdef __cplusplus
} /* extern "C" */

#endif

#endif /*__RECORDER_FP_ACTION_H__*/

