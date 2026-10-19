

/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file define the basic playservice struct and APIs.
 */
#ifndef __FP_PLAYSERVICE_H__
#define __FP_PLAYSERVICE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "aic_type.h"
#include "audio_system.h"
#include "audiofocus.h"
#include "aic_list.h"
#include "fp_playlist.h"

    struct playservice;

    enum ps_client_cb_event {
        /* PlayUI sync */
        CLIENT_EVENT_PS_PLAY_NEXT = 0,

        /* Error occrured, e.g. file read err */
        CLIENT_EVENT_PS_PLAY_ERR = 1,
        CLIENT_EVENT_PS_PLAY_FINISH,
        CLIENT_EVENT_PS_PLAY_TIME_SYNC,             /*Resv*/
        CLIENT_EVENT_PS_PLAY_PLAUSE,
        };

    typedef void(*playservice_notify_t) (enum ps_client_cb_event event,
        void * appdata);


    enum media_type {
        MEDIA_AUDIO,MEDIA_VIDEO,MEDIA_MAX
        };

    enum ps_event {
        EVENT_PS_PLAY_FINISH = 0,
        EVENT_PS_PLAY_BREAK = 1,
        EVENT_PS_SERV_STARTED = 2,
        EVENT_PS_PLAY_DURATION
        };

    enum ps_status {
        PS_IDLE,
        PS_PLAYING,
        PS_PAUSE,
        PS_STOP,
        };

    struct playservice_ops {
        u32(*play) (struct playservice * ps,struct playlist * pl,u32 start_track_id,
            enum app_id id,enum af_audio_context ctx,af_notify_t notify,
            void * appdata);
        u32(*next) (struct playservice * ps);
        u32(*prev) (struct playservice * ps);
        u32(*seek) (struct playservice * ps,u16 pos);
        u32(*pause) (struct playservice * ps);
        u32(*resume) (struct playservice * ps);
        u32(*stop) (struct playservice * ps);
        u32(*getvol) (struct playservice * ps);
        u32(*setvol) (struct playservice * ps,u32 level);
        struct media_track * (*getcurtrack) (struct playservice * ps);
        void(*regcb) (struct playservice * ps,enum app_id id,
            playservice_notify_t cb,void * appdata);

        /*unreg when app exit, background play will not notify app after unreg*/
        void(*unregcb) (struct playservice * ps,enum app_id id);
        u32(*getplaytime) (struct playservice * ps);

        /* active or idle */
        enum ps_status(*getstatus) (struct playservice * ps);
        };

    struct playservice_priv {
        enum ps_status status;
        struct playlist * cur_pl;
        struct playlist * last_pl; /* Resv */
        struct media_track * cur_track;
        ACLIENT         ahandle;
        u16             cur_virt_playtime;          /* playtime display */
        playservice_notify_t appnotify;
        void *          appdata;
        u16             session_token;              /* TBD:Resv */
        aos_thread_id   tid;
        aos_event_ptr   event;
        aos_queue_ptr   queue;
        uint32_t        pre_playtime;
        };

    struct playservice {
        // struct app_service base;

        /* ops will used by app, and exec in app's thread */
        struct playservice_ops * ops;
        struct playservice_priv priv;
        };

    struct playservice * app_srv_playservice_create(void);

    enum ps_queue_type {
        QUEUE_PS_TYPE_MUSIC = 0,
        QUEUE_PS_TYPE_CALL = 1,
        QUEUE_PS_TYPE_TTS,
        QUEUE_PS_TYPE_RECOD,
        QUEUE_PS_TYPE_MAX,
        };

    enum ps_queue_id {
        QUEUE_PS_ID_FINISH = 0,
        QUEUE_PS_ID_BREAK = 1,
        QUEUE_PS_ID_PLAY,                           //2
        QUEUE_PS_ID_PAUSE,                          //3
        QUEUE_PS_ID_RESUME,                         //4
        QUEUE_PS_ID_STOP,                           //5
        QUEUE_PS_ID_PLAY_PAUSE,                     //6
        QUEUE_PS_ID_NEXT,                           //7
        QUEUE_PS_ID_CALL_RING,                      //8
        QUEUE_PS_ID_CALL_RINGBACK_VOICE,            //9
        QUEUE_PS_ID_CALL_PAUSE_OTHER_APP,           //10
        QUEUE_PS_ID_PLAY_PAUSE_PATH,                //11
        QUEUE_PS_ID_CALL_RING_STOP,                 //12
        QUEUE_PS_ID_CALL_PLACE_LOOKUP,              //13
        QUEUE_PS_ID_MUSIC_PENDING,                  //14
        QUEUE_PS_ID_PLAY_MODE,                      //15
        QUEUE_PS_ID_SET_MUSIC_PATH,                 //16
        QUEUE_PS_ID_NEWS,                           //17
        QUEUE_PS_ID_POWRR_OFF_RING,                 //18
        QUEUE_PS_ID_TTS_PLAY=0x8000000,
        };

    typedef struct {
        uint32_t        msg_type;
        uint32_t        msg_id;
        void *          p_param;
        uint32_t        size;
        } ps_queue_param_t;

#define     PS_CFG_SERVER_QUEUE_NUM (20)

    uint32_t ps_data_msg_recv(ps_queue_param_t * p_app_msg,uint32_t wait_option);
    uint32_t ps_data_msg_send(ps_queue_param_t * p_app_msg,uint32_t wait_option);
    uint32_t ps_data_msg_send_call_ring(void * param,uint32_t size);
    void ps_data_msg_send_call_stop_other_voice();
    void ps_data_msg_send_call_play();
    void ps_data_msg_send_call_stop();
    void ps_data_msg_send_call_ringback_voice(int32_t voice_type,int32_t voice_state);
    uint32_t ps_data_msg_send_call_ring_stop();
    void set_playsetvice_ps(void * ps,uint32_t type);
    void set_playsetvice_fun(void * fun,uint32_t type);
    void ps_data_msg_send_tts(uint32_t msg_id);
    void ps_data_msg_send_music(struct playservice * ps,uint32_t msg_id);
    void ps_data_msg_send_record(uint32_t msg_id);
    void ps_data_msg_send_power_off_ring();
    uint8_t get_ring_play_status();

#ifdef __cplusplus
} /* extern "C" */

#endif

#endif /* __FP_PLAYSERVICE_H__ */

