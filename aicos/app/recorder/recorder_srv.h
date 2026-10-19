/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file define the basic recorder service struct and APIs.
 */

#ifndef __RECORDER_SRV_H__
#define __RECORDER_SRV_H__

#ifdef __cplusplus
extern "C" {
#endif
/*******************************************
 *              INCLUDES
 ******************************************/
#include "recorder_conf.h"
#ifdef WIN32
#include "recorder_simu_adpt.h"
#else
#include "aic_osa.h"
#include "aic_type.h"
#include "audio_system.h"
#endif

/*******************************************
 *              TYPEDEFS
 ******************************************/
struct recording_file {
    char *url;
    enum astream_format format;
};

enum recorder_state {
    RECD_IDLE,
    RECD_PLAY,
    RECD_RECORD,
    RECD_PAUSE,
    RECD_RESUME,
    RECD_STOP,
    RECD_ERROR
};

enum recorder_event { EVENT_RECD_FINISH = 0, EVENT_RECD_BREAK = 1 };

struct recorder_service {
    struct recording_file file;
    // priv data below
    enum recorder_state state;
    ACLIENT ahandle;
    aos_event_ptr event;
    aos_thread_id tid;
};

/*******************************************
 *              GLOBAL PROTOTYPES
 ******************************************/
struct recorder_service *app_srv_recording_init(void);
void app_srv_recording_deinit(struct recorder_service *srv);
u32 app_srv_recording_start(struct recorder_service *recd);
u32 app_srv_recording_pause(struct recorder_service *recd);
u32 app_srv_recording_resume(struct recorder_service *recd);
u32 app_srv_recording_stop(struct recorder_service *recd);

u32 app_srv_recording_play(struct recorder_service *recd);
u32 app_srv_recording_playstop(struct recorder_service *recd);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __RECORDER_SRV_H__ */