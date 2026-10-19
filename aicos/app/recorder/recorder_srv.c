/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implements the recorder service.
 */

/*******************************************
 *              INCLUDES
 ******************************************/
#include "recorder_srv.h"
#ifndef WIN32
#include "aic_bitmap.h"
#include "aic_osa_ex.h"
#endif

/*******************************************
 *              DEFINES
 ******************************************/
#define RECDSERVICE_STACK_SIZE 2048
#define RECDSERVICE_PRI 35

/*******************************************
 *              STATIC FUNCTIONS
 ******************************************/
static int recd_fileplay_cb(u32 data, enum astream_cb_event event)
{
    u32 flag = 0;
    struct recorder_service *recd = (struct recorder_service *)data;

    app_loge("recorder: recd_fileplay_cb enter event %d, data %u\n", event,
             data);

    if (event == ASTREAM_CB_EVENT_FINISH) {
        app_loge("recorder: recd_fileplay_cb ASTREAM_CB_EVENT_FINISH\n");
        flag = BIT(EVENT_RECD_FINISH);
    } else if (event == ASTREAM_CB_EVENT_BREAK) {
        flag = BIT(EVENT_RECD_BREAK);
    }

    if (flag) {
        aos_set_event(recd->event, flag, AOS_OR);
    }

    return 0;
}

static void recdservice(void *data)
{
    struct recorder_service *recd = data;
    u32 flag;
    aos_status status;

    while (1) {
        status = aos_get_event(recd->event,
                               BIT(EVENT_RECD_FINISH) | BIT(EVENT_RECD_BREAK),
                               AOS_OR_CLEAR, &flag, AOS_WAIT_FOREVER);
        if (status != AOS_SUCCESS) {
            app_loge("recdservice get event error\n");
        } else {
            if (flag & BIT(EVENT_RECD_FINISH)) {
                if (recd->state == RECD_PLAY) {
                    if (audio_stop(recd->ahandle)) {
                        app_loge("recorder: play stop failed\n");
                    }
                    if (audio_close(recd->ahandle)) {
                        app_loge("recorder: play close failed\n");
                    }

                    recd->state = RECD_IDLE;
                }
            } else if (flag & BIT(EVENT_RECD_BREAK)) {
            }
        }
    }
}

/*******************************************
 *              GLOBAL FUNCTIONS
 ******************************************/
struct recorder_service *app_srv_recording_init(void)
{
    struct recorder_service *srv;

    srv = aic_zalloc(sizeof(struct recorder_service));
    if (!srv) {
        app_loge("Recorder init malloc failed .\n");
        return NULL;
    }

    srv->event = aos_create_event("recdservice event");

    if (!srv->event) {
        aic_free(srv);
        app_loge("recorder service event create failed\n");
        return NULL;
    }

    srv->tid = aos_create_thread("recdservice", recdservice, (void *)srv,
                                 RECDSERVICE_STACK_SIZE, 0, NULL,
                                 RECDSERVICE_PRI, AOS_AUTO_START);

    srv->state = RECD_IDLE;
    srv->file.url = NULL;

    return srv;
}

void app_srv_recording_deinit(struct recorder_service *srv)
{
    aos_delete_event(srv->event);
    aos_delete_thread(srv->tid);
    aic_free(srv);
}

u32 app_srv_recording_start(struct recorder_service *recd)
{
    u32 ret;

    if (recd->state != RECD_IDLE) {
        app_loge("Recorder start status not IDLE .\n");
        return AOS_ERROR;
    }

    app_loge("recorder: url %s, foramt %d\n", recd->file.url,
             recd->file.format);

    recd->ahandle = audio_open_input(APLAY_TYPE_FILE, recd->file.format,
                                     APP_RECORDER_SAMPLE_RATE, ASTREAM_CHN_MONO,
                                     ASTREAM_SOURCE_MIC, 0);
    ret = audio_play_file(recd->ahandle, recd->file.url, recd_fileplay_cb,
                          (u32)recd);
    if (ret) {
        app_loge("Recorder start failed.\n");
        return AOS_ERROR;
    }
    recd->state = RECD_RECORD;

    return AOS_SUCCESS;
}

u32 app_srv_recording_pause(struct recorder_service *recd)
{
    if (recd->state != RECD_RECORD) {
        app_loge("Recorder pause faild (status not RECD_RECORD).\n");
        return AOS_ERROR;
    }

    if (audio_pause(recd->ahandle)) {
        app_loge("app_srv_recording_pause: pause failed\n");
        // return AOS_ERROR;
    }
    recd->state = RECD_PAUSE;
    return AOS_SUCCESS;
}

u32 app_srv_recording_resume(struct recorder_service *recd)
{
    if (recd->state != RECD_PAUSE) {
        app_loge("Recorder resume faild (status not RECD_PAUSE).\n");
        return AOS_ERROR;
    }

    if (audio_resume(recd->ahandle)) {
        app_loge("app_srv_recording_resume: resume failed\n");
        return AOS_ERROR;
    }
    recd->state = RECD_RECORD;
    return AOS_SUCCESS;
}

u32 app_srv_recording_stop(struct recorder_service *recd)
{
    if (recd->state != RECD_RECORD && recd->state != RECD_PAUSE) {
        app_loge("app_srv_recording_stop: Recorder stop faild (status not "
                 "RECD_PAUSE/RECD_RECORD).\n");
        return AOS_ERROR;
    }

    if (audio_stop(recd->ahandle)) {
        app_loge("app_srv_recording_stop: stop failed\n");
    }
    if (audio_close(recd->ahandle)) {
        app_loge("app_srv_recording_stop: close failed\n");
    }

    recd->state = RECD_IDLE;
    return AOS_SUCCESS;
}

u32 app_srv_recording_play(struct recorder_service *recd)
{
    app_loge("recorder: app_srv_recording_play  data %u, %p\n", (u32)recd,
             recd);
    recd->ahandle = audio_open_output(ASTREAM_TYPE_MUSIC, APLAY_TYPE_FILE,
                                      recd->file.format, 0);
    app_logi("play url %s, handle %u\n", recd->file.url, recd->ahandle);
    audio_play_file(recd->ahandle, recd->file.url, recd_fileplay_cb, (u32)recd);

    recd->state = RECD_PLAY;
    return AOS_SUCCESS;
}

u32 app_srv_recording_playstop(struct recorder_service *recd)
{
    if (audio_stop(recd->ahandle)) {
        app_loge("recorder: play stop failed\n");
        return AOS_ERROR;
    }
    if (audio_close(recd->ahandle)) {
        app_loge("recorder: play close failed\n");
        return AOS_ERROR;
    }
    recd->state = RECD_IDLE;
    return AOS_SUCCESS;
}