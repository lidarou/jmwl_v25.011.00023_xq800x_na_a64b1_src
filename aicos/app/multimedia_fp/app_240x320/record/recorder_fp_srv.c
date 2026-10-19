

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
#include "recorder_fp_srv.h"
#ifndef WIN32
#include "aic_bitmap.h"
#include "aic_osa_ex.h"
#endif

#include "../music/fp_playservice.h"

#include "recorder_fp_action.h"
#include "app_log.h"

#if 1
#define __inf                   APP_LOGI

#else

#define __inf                   0
#endif

/*******************************************
 *              DEFINES
 ******************************************/
#define RECDSERVICE_STACK_SIZE  2048
#define RECDSERVICE_PRI         35


struct recorder_service * g_recorder_srv = NULL;

/*******************************************
 *              STATIC FUNCTIONS
 ******************************************/
#if 0

static int recd_fileplay_cb(u32 data,enum astream_cb_event event)
{
    u32             flag = 0;

    struct recorder_service * recd = (struct recorder_service *) data;
    app_loge("recorder: recd_fileplay_cb enter event %d, data %u\n",event,
        data);

    if (event == ASTREAM_CB_EVENT_FINISH) {
        app_loge("recorder: recd_fileplay_cb ASTREAM_CB_EVENT_FINISH\n");
        flag                = BIT(EVENT_RECD_FINISH);
        }
    else if (event == ASTREAM_CB_EVENT_BREAK) {
        flag                = BIT(EVENT_RECD_BREAK);
        }

    if (flag) {
        aos_set_event(recd->event,flag,AOS_OR);
        }

    return 0;
}


static void recdservice(void * data)
{
    struct recorder_service * recd = data;
    u32             flag;
    aos_status      status;

    while (1) {
        status              = aos_get_event(recd->event,
            BIT(EVENT_RECD_FINISH) | BIT(EVENT_RECD_BREAK) | BIT(EVENT_RECD_PLAY_PAUSE),
            AOS_OR_CLEAR,&flag,AOS_WAIT_FOREVER);

        if (status != AOS_SUCCESS) {
            app_loge("recdservice get event error\n");
            }
        else {
            if (flag & BIT(EVENT_RECD_FINISH)) {
                if (recd->state == RECD_PLAY) {
                    if (audio_stop(recd->ahandle)) {
                        app_loge("recorder: play stop failed\n");
                        }

                    if (audio_close(recd->ahandle)) {
                        app_loge("recorder: play close failed\n");
                        }

                    recd->state         = RECD_IDLE;
                    app_recorder_service_notify_cb(RECD_IDLE);

                    //app_loge("recorder: EVENT_RECD_FINISH\n");
                    }
                }
            else if (flag & BIT(EVENT_RECD_BREAK)) {

                //app_loge("recorder: EVENT_RECD_BREAK\n");
                }
            else if (flag & BIT(EVENT_RECD_PLAY_PAUSE)) {
                app_recorder_play_pause_action();

                app_recorder_service_notify_cb(recd->state);

                //app_loge("recorder: EVENT_RECD_PLAY_PAUSE\n");
                }
            }
        }
}


/*******************************************
 *              GLOBAL FUNCTIONS
 ******************************************/
struct recorder_service * app_srv_recording_init(void) {
    struct recorder_service * srv;
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

    srv->tid = aos_create_thread("recdservice",recdservice,(void *) srv,
        RECDSERVICE_STACK_SIZE,0,NULL,
        RECDSERVICE_PRI,AOS_AUTO_START);

    srv->state = RECD_IDLE;
    srv->file.url = NULL;

    return srv;
} void app_srv_recording_deinit(struct recorder_service * srv)
{
    aos_delete_event(srv->event);
    aos_delete_thread(srv->tid);
    aic_free(srv);
}

#else


static int recd_fileplay_cb(u32 data,enum astream_cb_event event)
{
    if (event == ASTREAM_CB_EVENT_FINISH) {
        ps_data_msg_send_record(QUEUE_PS_ID_FINISH);
        }
    else if (event == ASTREAM_CB_EVENT_BREAK) {
        ps_data_msg_send_record(QUEUE_PS_ID_BREAK);
        }


    return AOS_SUCCESS;
}


void playservice_msg_record(struct recorder_service * recd,ps_queue_param_t * recv_msg)
{
    __inf("playservice_msg_record:%d,%d",recv_msg->msg_id,recd->state);

    switch (recv_msg->msg_id)
    {
        case QUEUE_PS_ID_FINISH:
            {
                if (recd->state == RECD_PLAY || recd->state == RECD_PAUSE) {
                    if (audio_stop(recd->ahandle)) {
                        app_loge("recorder: play stop failed\n");
                        }

                    if (audio_close(recd->ahandle)) {
                        app_loge("recorder: play close failed\n");
                        }

                    recd->state         = RECD_IDLE;
                    app_recorder_service_notify_cb(recv_msg->msg_id);

                    //app_loge("recorder: EVENT_RECD_FINISH\n");
                    }
                }
            break;

        case QUEUE_PS_ID_BREAK:
            {
                if (recd->state == RECD_PLAY || recd->state == RECD_PAUSE) {
                    if (audio_stop(recd->ahandle)) {
                        app_loge("recorder: play stop failed\n");
                        }

                    if (audio_close(recd->ahandle)) {
                        app_loge("recorder: play close failed\n");
                        }

                    recd->state         = RECD_IDLE;
                    app_recorder_service_notify_cb(recv_msg->msg_id);

                    //app_loge("recorder: EVENT_RECD_FINISH\n");
                    }
                }
            break;

        case QUEUE_PS_ID_PLAY:
            {
                app_recorder_setvol(app_recorder_getvol());
                app_recorder_play_action();
                app_recorder_service_notify_cb(recv_msg->msg_id);
                __inf("QUEUE_PS_ID_PLAY:%d,%d",recv_msg->msg_id,recd->state);
                }
            break;

        case QUEUE_PS_ID_PAUSE:
            {
                app_recorder_pause_action();

                app_recorder_service_notify_cb(recv_msg->msg_id);
                }
            break;

        case QUEUE_PS_ID_RESUME:
            if (recv_msg->size == 0)
                app_recorder_resume_action(recv_msg->p_param);
            else 
                app_recorder_call_resume_action(recv_msg->p_param);

            break;

        case QUEUE_PS_ID_STOP:
            {

                app_recorder_stop_action();
                }
            break;

        case QUEUE_PS_ID_PLAY_PAUSE:
            {
                app_recorder_setvol(app_recorder_getvol());
                app_recorder_play_pause_action();

                app_recorder_service_notify_cb(recv_msg->msg_id);

                //app_loge("recorder: EVENT_RECD_PLAY_PAUSE\n");
                }
            break;

        case QUEUE_PS_ID_NEXT:
            break;

        case QUEUE_PS_ID_PLAY_PAUSE_PATH:
            {
                app_recorder_setvol(app_recorder_getvol());
                app_recorder_play_pause_path_action(recv_msg->p_param);

                app_recorder_service_notify_cb(recv_msg->msg_id);
                }
            break;
        }
}


static struct recorder_service * g_recorder_service = NULL;

struct recorder_service * app_srv_recording_init(void) {
    struct recorder_service * srv;

    if (g_recorder_service != NULL)
        return g_recorder_service;

    srv = aic_zalloc(sizeof(struct recorder_service));

    if (!srv) {
        app_loge("Recorder init malloc failed .\n");
        return NULL;
        }

    set_playsetvice_ps(srv,QUEUE_PS_TYPE_RECOD);
    set_playsetvice_fun(playservice_msg_record,QUEUE_PS_TYPE_RECOD);

    srv->state = RECD_IDLE;
    srv->file.url = NULL;

    g_recorder_service = srv;

    return srv;
} void app_srv_recording_deinit(struct recorder_service * srv)
{
    if (srv->file.url) {
        aic_free(srv->file.url);
        srv->file.url       = NULL;
        }

    //aic_free(srv);
}

#endif


u32 app_srv_recording_start(struct recorder_service * recd)
{
    u32             ret;

    if (recd->state != RECD_IDLE) {
        app_loge("Recorder start status not IDLE .\n");
        return AOS_ERROR;
        }

    app_loge("recorder: url %s, foramt %d\n",recd->file.url,
        recd->file.format);

    recd->ahandle       = audio_open_input(APLAY_TYPE_FILE,recd->file.format,
        APP_RECORDER_SAMPLE_RATE,ASTREAM_CHN_MONO,
        ASTREAM_SOURCE_MIC,0);
    ret                 = audio_play_file(recd->ahandle,recd->file.url,recd_fileplay_cb,
        (u32) recd);

    if (ret) {
        app_loge("Recorder start failed.\n");
        return AOS_ERROR;
        }

    recd->state         = RECD_RECORD;

    return AOS_SUCCESS;
}


u32 app_srv_recording_call_start(struct recorder_service * recd)
{
    u32             ret;

    if (recd->state != RECD_IDLE) {
        app_loge("Recorder start status not IDLE .\n");
        return AOS_ERROR;
        }

    app_loge("recorder: url %s, foramt %d\n",recd->file.url,
        recd->file.format);

    recd->ahandle       = audio_open_input(APLAY_TYPE_FILE,recd->file.format,
        APP_RECORDER_SAMPLE_RATE,ASTREAM_CHN_MONO,
        ASTREAM_SOURCE_MIC,AUDIO_INPUT_FLAG_COMPRESS | AUDIO_INPUT_FLAG_VOREC);
    ret                 = audio_play_file(recd->ahandle,recd->file.url,recd_fileplay_cb,
        (u32) recd);

    if (ret) {
        app_loge("Recorder start failed.\n");
        return AOS_ERROR;
        }

    recd->state         = RECD_RECORD;

    return AOS_SUCCESS;
}


u32 app_srv_recording_pause(struct recorder_service * recd)
{
    if (recd->state != RECD_RECORD) {
        app_loge("Recorder pause faild (status not RECD_RECORD).\n");
        return AOS_ERROR;
        }

    if (audio_pause(recd->ahandle)) {
        app_loge("app_srv_recording_pause: pause failed\n");
        return AOS_ERROR;
        }

    recd->state         = RECD_PAUSE;
    return AOS_SUCCESS;
}


u32 app_srv_recording_resume(struct recorder_service * recd)
{
    if (recd->state != RECD_PAUSE) {
        app_loge("Recorder resume faild (status not RECD_PAUSE).\n");
        return AOS_ERROR;
        }

    if (audio_resume(recd->ahandle)) {
        app_loge("app_srv_recording_resume: resume failed\n");
        return AOS_ERROR;
        }

    recd->state         = RECD_RECORD;
    return AOS_SUCCESS;
}


u32 app_srv_recording_stop(struct recorder_service * recd)
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

    recd->state         = RECD_IDLE;
    return AOS_SUCCESS;
}


u32 app_srv_recording_play(struct recorder_service * recd)
{
    u32             ret = AOS_ERROR;

    app_loge("recorder: app_srv_recording_play  data %u, %p\n",(u32) recd,
        recd);
    recd->ahandle       = audio_open_output(ASTREAM_TYPE_MUSIC,APLAY_TYPE_FILE,
        recd->file.format,0);
    app_logi("play url %s, handle %u\n",recd->file.url,recd->ahandle);
    ret                 = audio_play_file(recd->ahandle,recd->file.url,recd_fileplay_cb,(u32) recd);

    if (ret) {
        app_logw("recorder audio_play_file  failed\n");
        audio_close(recd->ahandle);
        return AOS_ERROR;

        }

    recd->state         = RECD_PLAY;
    return AOS_SUCCESS;
}


u32 app_srv_recording_playpause(struct recorder_service * recd)
{
    if ((recd->state != RECD_PLAY) && (recd->state != RECD_PAUSE)) {
        app_loge("Recorder pause faild (status not RECD_RECORD).\n");
        return AOS_ERROR;
        }

    if (recd->state == RECD_PLAY) {
        if (audio_pause(recd->ahandle)) {
            app_loge("app_srv_recording_pause: pause failed\n");
            return AOS_ERROR;
            }

        recd->state         = RECD_PAUSE;
        }
    else if (recd->state == RECD_PAUSE) {
        if (audio_resume(recd->ahandle)) {
            app_loge("app_srv_recording_resume: resume failed\n");
            return AOS_ERROR;
            }

        recd->state         = RECD_PLAY;
        }

    return AOS_SUCCESS;
}


u32 app_srv_recording_playstop(struct recorder_service * recd)
{
    if (audio_stop(recd->ahandle)) {
        app_loge("recorder: play stop failed\n");

        //return AOS_ERROR;
        }

    if (audio_close(recd->ahandle)) {
        app_loge("recorder: play close failed\n");
        return AOS_ERROR;
        }

    recd->state         = RECD_IDLE;
    return AOS_SUCCESS;
}


